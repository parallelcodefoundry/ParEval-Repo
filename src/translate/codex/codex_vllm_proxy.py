"""Lightweight proxy that sits between Codex CLI and vLLM.

Rewrites tool types that vLLM's /v1/responses endpoint does not support:
  - "custom"      -> "function"  (freeform grammar tools like apply_patch)
  - "local_shell" -> "function"  (built-in shell tool)
  - strips "web_search" / "image_generation" (unsupported, rarely needed)

On the response path it converts function_call items back to the format
Codex expects (custom_tool_call / local_shell_call).

Usage:
    python codex_vllm_proxy.py [--vllm-url http://127.0.0.1:8008] [--port 9000]

Then point Codex at the proxy:
    export OPENAI_BASE_URL="http://127.0.0.1:9000/v1"
"""

import argparse
import json
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.request import Request, urlopen
from urllib.error import HTTPError

# ---------------------------------------------------------------------------
# Tool rewriting helpers
# ---------------------------------------------------------------------------

# Track which tools were rewritten so we can fix the response
_rewritten_tools = {}  # name -> original type


def rewrite_request_input(body):
    """Sanitize the 'input' conversation history for vLLM compatibility.

    On multi-turn requests Codex sends back the full conversation history
    including item types that vLLM's /v1/responses endpoint cannot parse
    (e.g. reasoning items, custom_tool_call, custom_tool_call_output,
    local_shell_call, local_shell_call_output).  We convert or strip
    these so that vLLM only sees types it understands.
    """
    if "input" not in body or not isinstance(body["input"], list):
        return body

    new_input = []
    for item in body["input"]:
        if not isinstance(item, dict):
            new_input.append(item)
            continue

        item_type = item.get("type", "")

        # --- reasoning items: strip entirely (not needed for generation) ---
        if item_type == "reasoning":
            continue

        # --- custom_tool_call -> function_call ---
        if item_type == "custom_tool_call":
            raw_input = item.get("input", "")
            new_input.append({
                "type": "function_call",
                "call_id": item.get("call_id", ""),
                "name": item.get("name", ""),
                "arguments": json.dumps({"input": raw_input}),
            })
            continue

        # --- custom_tool_call_output -> function_call_output ---
        if item_type == "custom_tool_call_output":
            new_input.append({
                "type": "function_call_output",
                "call_id": item.get("call_id", ""),
                "output": item.get("output", ""),
            })
            continue

        # --- local_shell_call -> function_call ---
        if item_type == "local_shell_call":
            action = item.get("action", {})
            args = {}
            if "command" in action:
                args["command"] = action["command"]
            if "workdir" in action:
                args["workdir"] = action["workdir"]
            if "timeout_ms" in action:
                args["timeout_ms"] = action["timeout_ms"]
            new_input.append({
                "type": "function_call",
                "call_id": item.get("call_id", ""),
                "name": "local_shell",
                "arguments": json.dumps(args),
            })
            continue

        # --- local_shell_call_output -> function_call_output ---
        if item_type == "local_shell_call_output":
            new_input.append({
                "type": "function_call_output",
                "call_id": item.get("call_id", ""),
                "output": item.get("output", ""),
            })
            continue

        # --- message items: strip encrypted_content / reasoning_text from content ---
        if item_type == "message" and isinstance(item.get("content"), list):
            cleaned = item.copy()
            cleaned["content"] = [
                c for c in item["content"]
                if not (isinstance(c, dict) and c.get("type") == "reasoning_text")
            ]
            new_input.append(cleaned)
            continue

        # --- pass through everything else unchanged ---
        new_input.append(item)

    body["input"] = new_input
    return body


def rewrite_request_tools(body):
    """Rewrite tools in the request body to types vLLM supports."""
    _rewritten_tools.clear()

    if "tools" not in body:
        return body

    new_tools = []
    for tool in body["tools"]:
        tool_type = tool.get("type", "")

        if tool_type == "custom":
            # Convert freeform/grammar tool to a function tool with a
            # single string "input" parameter.  The model should still
            # produce the same grammar-formatted text as the "input" value.
            _rewritten_tools[tool["name"]] = "custom"
            new_tools.append({
                "type": "function",
                "name": tool["name"],
                "description": tool.get("description", ""),
                "parameters": {
                    "type": "object",
                    "properties": {
                        "input": {
                            "type": "string",
                            "description": "The freeform tool input.",
                        }
                    },
                    "required": ["input"],
                    "additionalProperties": False,
                },
            })

        elif tool_type == "local_shell":
            # Convert built-in local_shell to a function tool matching
            # Codex's own "shell" function schema.
            _rewritten_tools["local_shell"] = "local_shell"
            new_tools.append({
                "type": "function",
                "name": "local_shell",
                "description": "Run a shell command and return its output.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "command": {
                            "type": "array",
                            "items": {"type": "string"},
                            "description": "The command to execute.",
                        },
                        "workdir": {
                            "type": "string",
                            "description": "Working directory.",
                        },
                        "timeout_ms": {
                            "type": "number",
                            "description": "Timeout in milliseconds.",
                        },
                    },
                    "required": ["command"],
                    "additionalProperties": False,
                },
            })

        elif tool_type in ("web_search", "image_generation"):
            # Strip unsupported built-in types
            continue

        else:
            # Pass through function tools and anything else unchanged
            new_tools.append(tool)

    body["tools"] = new_tools
    return body


def rewrite_response_output(body):
    """Rewrite function_call items back to custom_tool_call / local_shell_call
    for tools that were originally non-function types."""
    if "output" not in body:
        return body

    new_output = []
    for item in body["output"]:
        item_type = item.get("type", "")

        if item_type == "function_call":
            name = item.get("name", "")
            original_type = _rewritten_tools.get(name)

            if original_type == "custom":
                # Extract the raw input string from the function arguments
                try:
                    args = json.loads(item.get("arguments", "{}"))
                    raw_input = args.get("input", item.get("arguments", ""))
                except (json.JSONDecodeError, TypeError):
                    raw_input = item.get("arguments", "")

                new_output.append({
                    "type": "custom_tool_call",
                    "call_id": item.get("call_id", item.get("id", "")),
                    "name": name,
                    "input": raw_input,
                })
                continue

            elif original_type == "local_shell":
                try:
                    args = json.loads(item.get("arguments", "{}"))
                except (json.JSONDecodeError, TypeError):
                    args = {}

                action = {"type": "exec"}
                if "command" in args:
                    action["command"] = args["command"]
                if "workdir" in args:
                    action["workdir"] = args["workdir"]
                if "timeout_ms" in args:
                    action["timeout_ms"] = args["timeout_ms"]

                new_output.append({
                    "type": "local_shell_call",
                    "call_id": item.get("call_id", item.get("id", "")),
                    "action": action,
                    "status": item.get("status", "completed"),
                })
                continue

        new_output.append(item)

    body["output"] = new_output
    return body


# ---------------------------------------------------------------------------
# HTTP Proxy
# ---------------------------------------------------------------------------

class CodexVLLMProxy(BaseHTTPRequestHandler):
    vllm_url = "http://127.0.0.1:8008"

    def _proxy(self, method="GET", body=None):
        target_url = self.vllm_url + self.path

        headers = {"Content-Type": "application/json"}
        for key in ("Authorization", "Accept"):
            val = self.headers.get(key)
            if val:
                headers[key] = val

        # Rewrite request body if needed
        if body and self.path.endswith("/responses"):
            try:
                data = json.loads(body)
                data = rewrite_request_input(data)
                data = rewrite_request_tools(data)
                body = json.dumps(data).encode()
            except (json.JSONDecodeError, TypeError):
                pass

        req = Request(target_url, data=body, headers=headers, method=method)

        try:
            resp = urlopen(req, timeout=600)
            resp_body = resp.read()
        except HTTPError as e:
            resp_body = e.read()
            self.send_response(e.code)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(resp_body)
            return

        # Rewrite response body if needed
        if self.path.endswith("/responses") and _rewritten_tools:
            try:
                resp_data = json.loads(resp_body)
                resp_data = rewrite_response_output(resp_data)
                resp_body = json.dumps(resp_data).encode()
            except (json.JSONDecodeError, TypeError):
                pass

        self.send_response(resp.status)
        self.send_header("Content-Type", resp.headers.get("Content-Type", "application/json"))
        self.end_headers()
        self.wfile.write(resp_body)

    def do_GET(self):
        self._proxy("GET")

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length) if length else None
        self._proxy("POST", body)

    def log_message(self, format, *args):
        # Prefix log lines for clarity
        sys.stderr.write(f"[codex-proxy] {format % args}\n")


def main():
    parser = argparse.ArgumentParser(description="Codex <-> vLLM proxy")
    parser.add_argument("--vllm-url", default="http://127.0.0.1:8008",
                        help="Base URL of the vLLM server (default: http://127.0.0.1:8008)")
    parser.add_argument("--port", type=int, default=9000,
                        help="Port for the proxy to listen on (default: 9000)")
    parser.add_argument("--host", default="127.0.0.1",
                        help="Host for the proxy to bind to (default: 127.0.0.1)")
    args = parser.parse_args()

    CodexVLLMProxy.vllm_url = args.vllm_url
    server = HTTPServer((args.host, args.port), CodexVLLMProxy)
    print(f"Codex-vLLM proxy listening on http://{args.host}:{args.port}")
    print(f"Forwarding to vLLM at {args.vllm_url}")
    print(f"Set: export OPENAI_BASE_URL=\"http://{args.host}:{args.port}/v1\"")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nProxy stopped.")
        server.server_close()


if __name__ == "__main__":
    main()
