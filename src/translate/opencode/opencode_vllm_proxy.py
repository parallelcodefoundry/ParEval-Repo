"""Lightweight proxy that sits between OpenCode and vLLM.

Caps max_tokens on /v1/chat/completions requests so OpenCode's internal
default of 32000 doesn't exceed the model's total context window.

Usage:
    python opencode_vllm_proxy.py [--vllm-url http://127.0.0.1:8008] \
                                  [--port 9001] [--max-tokens 16384]

Then point OpenCode at the proxy:
    --opencode-provider-base-url http://127.0.0.1:9001/v1
"""

import argparse
import json
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.request import Request, urlopen
from urllib.error import HTTPError


MAX_TOKENS_CAP = 16384


class OpenCodeVLLMProxy(BaseHTTPRequestHandler):
    vllm_url = "http://127.0.0.1:8008"
    max_tokens_cap = MAX_TOKENS_CAP

    def _proxy(self, method="GET", body=None):
        target_url = self.vllm_url + self.path

        headers = {"Content-Type": "application/json"}
        for key in ("Authorization", "Accept"):
            val = self.headers.get(key)
            if val:
                headers[key] = val

        if body and self.path.endswith("/chat/completions"):
            try:
                data = json.loads(body)
                current = data.get("max_tokens")
                if current is None or current > self.max_tokens_cap:
                    data["max_tokens"] = self.max_tokens_cap
                body = json.dumps(data).encode()
            except (json.JSONDecodeError, TypeError):
                pass

        req = Request(target_url, data=body, headers=headers, method=method)

        try:
            resp = urlopen(req, timeout=600)
            resp_body = resp.read()
            status = resp.status
            content_type = resp.headers.get("Content-Type", "application/json")
        except HTTPError as e:
            resp_body = e.read()
            self.send_response(e.code)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(resp_body)
            return

        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.end_headers()
        self.wfile.write(resp_body)

    def do_GET(self):
        self._proxy("GET")

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length) if length else None
        self._proxy("POST", body)

    def log_message(self, format, *args):
        sys.stderr.write(f"[opencode-proxy] {format % args}\n")


def main():
    parser = argparse.ArgumentParser(description="OpenCode <-> vLLM proxy")
    parser.add_argument("--vllm-url", default="http://127.0.0.1:8008",
                        help="Base URL of the vLLM server (default: http://127.0.0.1:8008)")
    parser.add_argument("--port", type=int, default=9001,
                        help="Port for the proxy to listen on (default: 9001)")
    parser.add_argument("--host", default="127.0.0.1",
                        help="Host for the proxy to bind to (default: 127.0.0.1)")
    parser.add_argument("--max-tokens", type=int, default=MAX_TOKENS_CAP,
                        help=f"Cap max_tokens on chat/completions requests (default: {MAX_TOKENS_CAP})")
    args = parser.parse_args()

    OpenCodeVLLMProxy.vllm_url = args.vllm_url
    OpenCodeVLLMProxy.max_tokens_cap = args.max_tokens
    server = HTTPServer((args.host, args.port), OpenCodeVLLMProxy)
    print(f"OpenCode-vLLM proxy listening on http://{args.host}:{args.port}")
    print(f"Forwarding to vLLM at {args.vllm_url} (max_tokens capped at {args.max_tokens})")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nProxy stopped.")
        server.server_close()


if __name__ == "__main__":
    main()
