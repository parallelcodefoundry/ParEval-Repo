""" Mixin class for providing a unified generative interface within other classes.

    author: Daniel Nichols
    date: November 2024
"""
import logging
import os
import sys
import time
import pickle
import atexit
import subprocess
import requests
from pathlib import Path
from typing import Literal, Callable, Any
from math import ceil
from dataclasses import dataclass

logger = logging.getLogger("pareval-repo")

@dataclass
class GenericResponse:
    """ Class to hold a generic response from any generator.
    """
    response: str
    prompt_tokens: float
    completion_tokens: float
    reasoning: str | None = None


# Constants
MAX_ATTEMPTS = 3
DEFAULT_OPENAI_RPM = 100
DEFAULT_OPENAI_TPM = 150000
DEFAULT_GEMINI_RPM = 7
DEFAULT_GEMINI_TPM = 6000
VLLM_BASE_URL = os.environ.get("OPENAI_API_BASE", "http://127.0.0.1:8000/v1")
VLLM_API_KEY = "token_abc123"
VLLM_TIMEOUT = 1200
BASE_COOLDOWN = 10
CACHE_RETENTION_TIME = 60
VLLM_SERVE_CHECK_COOLDOWN = 10
VLLM_MAX_SERVE_CHECK_ATTEMPTS = 200
VLLM_MAX_TIMEOUT_RETRIES = 3


@dataclass
class BackendConfig:
    """Configuration for different backends."""
    rpm_limit: int | None = None
    tpm_limit: int | None = None
    requires_api_key: bool = False
    api_key_env_var: str | None = None


class GeneratorMixin:
    """ Mixin class for providing a unified generative interface within other classes.
    """

    # Token limit constants (match keet's values)
    _REASONING_OUTPUT_RESERVE: int = 32000  # tokens reserved for reasoning and output
    _MAX_TOKEN_COUNT: int = 131072          # 128k tokens, limit for most models

    _backend: Literal["openai", "gemini", "hf", "vllm", "local"]
    _llm_name: str
    _generator: Callable | None = None
    _rpm_limit: int | None = None
    _tpm_limit: int | None = None
    _recent_requests: list[tuple[float, int]] = []
    _max_input_tokens: int | None = None
    _max_output_tokens: int | None = None
    _max_requests: int | None = None
    _input_token_count: int = 0
    _output_token_count: int = 0
    _request_count: int = 0
    _system_prompt: str | None = None
    _disable_request_cache: bool | None = False

    _openai_client: 'OpenAI' | None = None  # type: ignore # noqa: F821
    _gemini_client: "GenerativeAI" | None = None  # type: ignore # noqa: F821
    _hf_inference_client: "InferenceClient" | None = None  # type: ignore # noqa: F821
    _vllm_client: 'OpenAI' | None = None  # type: ignore # noqa: F821
    _async_mode: bool = False
    _vllm_environment: str | None = None
    _vllm_yaml_config: str | None = None
    _vllm_keepalive_id: str | None = None
    _api_key: str | None = None
    _api_base_url: str | None = None
    _encoder: Any | None = None

    # Backend configurations
    _BACKEND_CONFIGS = {
        "openai": BackendConfig(
            rpm_limit=DEFAULT_OPENAI_RPM,
            tpm_limit=DEFAULT_OPENAI_TPM,
            requires_api_key=False
        ),
        "gemini": BackendConfig(
            rpm_limit=DEFAULT_GEMINI_RPM,
            tpm_limit=DEFAULT_GEMINI_TPM,
            requires_api_key=True,
            api_key_env_var="GEMINI_API_KEY"
        ),
        "hf": BackendConfig(
            requires_api_key=True,
            api_key_env_var="HF_API_KEY"
        ),
        "vllm": BackendConfig(),
        "local": BackendConfig()
    }


    def __init__(
        self,
        backend: Literal["openai", "gemini", "hf", "vllm", "local"],
        llm_name: str,
        rpm_limit: int | None = None,
        tpm_limit: int | None = None,
        max_input_tokens: int | None = None,
        max_output_tokens: int | None = None,
        max_requests: int | None = None,
        system_prompt: str | None = None,
        disable_request_cache: bool | None = False,
        async_mode: bool = False,
        vllm_environment: str | None = None,
        vllm_yaml_config: str | None = None,
        vllm_keepalive_id: str | None = None,
        api_key: str | None = None,
        api_base_url: str | None = None,
    ):
        self._backend = backend
        self._llm_name = llm_name
        self._max_input_tokens = max_input_tokens
        self._max_output_tokens = max_output_tokens
        self._max_requests = max_requests
        self._system_prompt = system_prompt
        self._disable_request_cache = disable_request_cache
        self._async_mode = async_mode
        self._vllm_environment = vllm_environment
        self._vllm_yaml_config = vllm_yaml_config
        self._vllm_keepalive_id = vllm_keepalive_id
        self._api_key = api_key
        self._api_base_url = api_base_url
        self._encoder = None

        self._validate_backend()
        self._configure_backend(rpm_limit, tpm_limit)
        self._setup_rate_limiting()
        self._setup_encoder()


    def _validate_backend(self) -> None:
        """Validate that the backend is supported."""
        if self._backend not in self._BACKEND_CONFIGS:
            raise ValueError(f"Invalid backend specified: '{self._backend}'")


    def _configure_backend(self, rpm_limit: int | None, tpm_limit: int | None) -> None:
        """Configure the backend client and generator."""
        config = self._BACKEND_CONFIGS[self._backend]

        # Set rate limits from config if not provided
        self._rpm_limit = rpm_limit or config.rpm_limit
        self._tpm_limit = tpm_limit or config.tpm_limit

        # Check API key requirements (skip if a generic api_key was provided)
        if config.requires_api_key and config.api_key_env_var and self._api_key is None:
            if not os.environ.get(config.api_key_env_var):
                raise ValueError(f"{config.api_key_env_var} environment variable not set.")

        # Initialize backend-specific clients and generators
        if self._backend == "openai":
            self._setup_openai()
        elif self._backend == "vllm":
            self._setup_vllm()
        elif self._backend == "gemini":
            self._setup_gemini()
        elif self._backend == "hf":
            self._setup_huggingface()
        else:
            raise NotImplementedError(f"backend '{self._backend}' not implemented.")


    def _setup_encoder(self) -> None:
        """Set up a tiktoken encoder for token counting if the backend supports it."""
        if self._backend not in ("openai", "vllm"):
            return
        try:
            import tiktoken
            query_str = self._llm_name
            if "/" in query_str:
                query_str = query_str.split("/")[-1]
            self._encoder = tiktoken.encoding_for_model(query_str)
        except (KeyError, ValueError):
            # Model not in tiktoken's registry; get_token_count will use a default encoding
            self._encoder = None


    def _setup_openai(self) -> None:
        """Setup OpenAI client and generator."""
        from openai import OpenAI
        self._openai_client = OpenAI(
            api_key=self._api_key,  # None → SDK reads OPENAI_API_KEY env var
            base_url=self._api_base_url,  # None → SDK uses default
        )
        self._generator = self._generate_openai


    def _setup_vllm(self) -> None:
        """Setup vLLM client and generator."""
        if self._async_mode:
            from openai import AsyncOpenAI as OpenAI
        else:
            from openai import OpenAI # type: ignore

        api_key = self._api_key or os.environ.get("VLLM_API_KEY", VLLM_API_KEY)
        base_url = self._api_base_url or VLLM_BASE_URL

        self._vllm_client = OpenAI(
            base_url=base_url,
            api_key=api_key,
            timeout=VLLM_TIMEOUT
        )
        self._generator = self._generate_vllm

        if self._vllm_environment:
            self._launch_vllm_server(
                Path(self._vllm_environment),
                self._vllm_yaml_config,
                self._vllm_keepalive_id,
            )
        elif not self._test_vllm_server():
            self._wait_for_vllm_server()


    def _test_vllm_server(self) -> bool:
        """Test if the vLLM server is running."""
        base = (self._api_base_url or VLLM_BASE_URL).rstrip("/")
        if base.endswith("/v1"):
            base = base[:-3]
        health_url = base + "/health"
        try:
            response = requests.get(health_url, timeout=5)
            return response.ok
        except requests.RequestException:
            return False


    def _wait_for_vllm_server(self) -> None:
        """Poll until the vLLM server is ready or max attempts are exceeded."""
        num_attempts = 0
        while num_attempts < VLLM_MAX_SERVE_CHECK_ATTEMPTS:
            if self._test_vllm_server():
                return
            logger.info("vLLM server not ready, checking again after %d seconds...", VLLM_SERVE_CHECK_COOLDOWN)
            time.sleep(VLLM_SERVE_CHECK_COOLDOWN)
            num_attempts += 1
        raise RuntimeError("vLLM server not ready after max attempts.")


    def _launch_vllm_server(
        self,
        environment_path: Path,
        yaml_config: str | None = None,
        keepalive_id: str | None = None,
    ) -> subprocess.Popen | None:
        """Launch a vLLM server in the background using the given Python environment.

        Args:
            environment_path: Path to the Python virtual environment directory.
            yaml_config: Optional path to a vLLM YAML configuration file.
            keepalive_id: If set, write the server PID to a file instead of registering
                an atexit handler so the process can outlive this Python session.

        Returns:
            The Popen object for the vLLM server, or None if the server was already running.
        """
        if self._test_vllm_server():
            return None

        py_executable = environment_path / "bin" / "python"
        vllm_executable = environment_path / "bin" / "vllm"

        api_key = self._api_key or os.environ.get("VLLM_API_KEY", VLLM_API_KEY)
        vllm_command: list[str] = [
            str(py_executable),
            str(vllm_executable),
            "serve",
            "--model", self._llm_name,
            "--host", "127.0.0.1",
            "--port", "8000"
        ]
        if api_key:
            vllm_command.extend(["--api-key", api_key])
        if self._is_reasoning_model():
            vllm_command.extend(["--reasoning-parser", "deepseek_r1"])
        if yaml_config:
            vllm_command.extend(["--config", yaml_config])

        logger.info("Launching vLLM server: %s", " ".join(vllm_command))
        vllm_server = subprocess.Popen(
            vllm_command,
            start_new_session=(keepalive_id is not None),
        )

        self._wait_for_vllm_server()

        if keepalive_id is not None:
            pid_file = Path(sys.argv[0]).resolve().parent / f"vllm-server-{keepalive_id}.pid"
            pid_file.write_text(str(vllm_server.pid), encoding="utf-8")
        else:
            atexit.register(vllm_server.terminate)

        logger.info("vLLM server ready.")
        return vllm_server


    def _setup_gemini(self) -> None:
        """Setup Gemini client and generator."""
        import google.generativeai as genai
        api_key = self._api_key or os.environ["GEMINI_API_KEY"]
        genai.configure(api_key=api_key)
        self._gemini_client = genai.GenerativeModel(
            self._llm_name,
            system_instruction=self._system_prompt
        )
        self._generator = self._generate_gemini


    def _setup_huggingface(self) -> None:
        """Setup Hugging Face client and generator."""
        from huggingface_hub import InferenceClient
        api_key = self._api_key or os.environ["HF_API_KEY"]
        self._hf_inference_client = InferenceClient(api_key=api_key)
        self._generator = self._generate_hf


    def _setup_rate_limiting(self) -> None:
        """Setup rate limiting and request caching."""
        if self._rpm_limit is not None or self._tpm_limit is not None:
            self._recent_requests = []
            self._load_request_cache()
            if not self._disable_request_cache:
                atexit.register(self._cleanup)


    def _load_request_cache(self) -> None:
        """Load recent requests from cache file."""
        if self._disable_request_cache:
            return

        try:
            with open(f".request_cache_{self._backend}.pkl", "rb") as f:
                self._recent_requests = pickle.load(f)
                logger.debug("Loaded %d recent requests from cache.", len(self._recent_requests or []))
        except FileNotFoundError:
            pass


    def _cleanup(self):
        """ Save recent requests to cache on deletion.
        """
        if len(self._recent_requests) > 0 and not self._disable_request_cache:
            if time.time() - self._recent_requests[-1][0] <= 60:
                with open(f".request_cache_{self._backend}.pkl", "wb") as f:
                    pickle.dump(self._recent_requests, f)


    def _format_messages_list(self, prompt: str,
                              system_prompt: str | None) \
                              -> list[dict[str, str]]:
        """ Format the prompt and system prompt into a list of messages for OpenAI.
        """
        messages = []
        if system_prompt is not None:
            messages.append({"role": "system", "content": system_prompt})
        messages.append({"role": "user", "content": prompt})
        return messages


    def _is_reasoning_model(self) -> bool:
        """Check if the current model is a reasoning model."""
        name_lower = self._llm_name.lower()
        return any(m in name_lower for m in ["qwq", "deepseek_r1", "deepseek-r1"])


    def _generate_openai(self, prompt: str, system_prompt: str, **kwargs) -> list[GenericResponse]:
        """ Generate text using the OpenAI responses API.
        """
        if not self._openai_client:
            raise ValueError("OpenAI client not initialized.")

        messages = self._format_messages_list(prompt, system_prompt)
        gen_kwargs: dict[str, Any] = {"model": self._llm_name, "input": messages, **kwargs}
        if self._llm_name.endswith("-thinking"):
            gen_kwargs["model"] = self._llm_name.replace("-thinking", "")
            gen_kwargs["reasoning"] = {"effort": "high"}

        response = self._openai_client.responses.create(**gen_kwargs)

        return [GenericResponse(
            response.output_text,
            response.usage.input_tokens,
            response.usage.output_tokens,
        )]


    def _generate_vllm(self, prompt: str, system_prompt: str, **kwargs) -> list[GenericResponse]:
        """ Generate text using the vLLM via the OpenAI responses API.
        """
        from openai import APITimeoutError
        if not self._vllm_client:
            raise ValueError("vLLM (OpenAI) client not initialized.")

        messages = self._format_messages_list(prompt, system_prompt)
        gen_kwargs: dict[str, Any] = {"model": self._llm_name, "input": messages, **kwargs}
        if "gpt-oss" in self._llm_name:
            gen_kwargs["reasoning"] = {"effort": "high"}

        for _ in range(VLLM_MAX_TIMEOUT_RETRIES):
            try:
                response = self._vllm_client.responses.create(**gen_kwargs)
                break
            except (TimeoutError, APITimeoutError) as e:
                logger.warning("vLLM request timeout (%s), retrying...", e)
                time.sleep(BASE_COOLDOWN)
                continue
        else:
            raise RuntimeError(
                f"vLLM request failed after {VLLM_MAX_TIMEOUT_RETRIES} timeout retries."
            )

        return [GenericResponse(
            response.output_text,
            response.usage.input_tokens if response.usage else 0,
            response.usage.output_tokens if response.usage else 0,
        )]


    def _parse_reasoning_response(self, response: str) -> tuple[str, str | None]:
        """Parse reasoning response to extract reasoning and content."""
        if "</think>" in response:
            parts = response.split("</think>")
            return parts[1] if len(parts) > 1 else "", parts[0]
        return response, response


    async def _generate_vllm_async(
            self,
            prompts: list[str],
            system_prompt: str | None,
            **kwargs
    ) -> list[GenericResponse]:
        """Generate text using vLLM via the OpenAI Responses API in async batch mode.
        """
        import asyncio
        if not self._vllm_client:
            raise ValueError("vLLM (OpenAI) client not initialized.")

        async def _generate_single(prompt: str) -> GenericResponse:
            messages = self._format_messages_list(prompt, system_prompt)
            gen_kwargs: dict[str, Any] = {"model": self._llm_name, "input": messages, **kwargs}
            if "gpt-oss" in self._llm_name:
                gen_kwargs["reasoning"] = {"effort": "high"}

            response_obj = await self._vllm_client.responses.create(**gen_kwargs)

            response_text, reasoning = response_obj.output_text, None
            if self._is_reasoning_model():
                response_text, reasoning = self._parse_reasoning_response(response_text)

            return GenericResponse(
                response_text,
                response_obj.usage.input_tokens if response_obj.usage else 0,
                response_obj.usage.output_tokens if response_obj.usage else 0,
                reasoning
            )

        results = await asyncio.gather(*(_generate_single(prompt) for prompt in prompts))

        return results


    def _generate_gemini(self, prompt: str, system_prompt: str, **kwargs) -> list[GenericResponse]:
        """ Generate text using the Gemini API.
        """
        import google.generativeai as genai
        if not self._gemini_client:
            raise ValueError("Gemini client not initialized.")

        if system_prompt is not None and system_prompt != self._system_prompt:
            # System prompt can only be changed at model initialization, so we
            # would have to reinitialize the model for each generation.
            # We can implement this in the future if necessary.
            raise NotImplementedError("System prompt can only be changed at init with Gemini.")

        response = self._gemini_client.generate_content(
            prompt,
            generation_config=genai.types.GenerationConfig(candidate_count=1),
        )
        return [GenericResponse(response.text,
                                response.usage_metadata.prompt_token_count,
                                response.usage_metadata.candidates_token_count)]


    def _generate_hf(self, prompt: str, system_prompt: str, **kwargs) -> list[GenericResponse]:
        """ Generate text using the Hugging Face API.
        """
        if not self._hf_inference_client:
            raise ValueError("Hugging Face Inference client not initialized.")

        response = self._hf_inference_client.chat.completions.create(
            model=self._llm_name,
            messages=self._format_messages_list(prompt, system_prompt),
            **kwargs
        )

        if not response.choices:
            raise ValueError("No completions returned from Hugging Face.")

        return [GenericResponse(response.choices[0].message.content,
                                response.usage.prompt_tokens,
                                response.usage.completion_tokens)]


    def get_token_count(self, prompt: str, default_encoding: str = "o200k_base") -> int:
        """Return the number of tokens in the prompt according to tiktoken.

        Uses the model-specific encoding if available, falling back to default_encoding.
        """
        import tiktoken
        if self._encoder is None:
            encoder = tiktoken.get_encoding(default_encoding)
            return len(encoder.encode(prompt, disallowed_special=()))
        return len(self._encoder.encode(prompt, disallowed_special=()))


    def get_max_tokens(self) -> int:
        """Return the effective maximum number of input tokens for the current model.

        Reserves _REASONING_OUTPUT_RESERVE tokens from the true maximum for reasoning
        and output, matching keet's implementation.
        """
        if "gpt-5" in self._llm_name:
            if "thinking" in self._llm_name:
                return 196000 - self._REASONING_OUTPUT_RESERVE
            return 272000 - self._REASONING_OUTPUT_RESERVE
        return self._MAX_TOKEN_COUNT - self._REASONING_OUTPUT_RESERVE


    @property
    def input_token_count(self):
        """ Return the total number of tokens used as input to the generator.
        """
        return self._input_token_count


    @property
    def output_token_count(self):
        """ Return the total number of tokens generated by the generator.
        """
        return self._output_token_count


    @property
    def token_count(self):
        """ Return the total number of tokens used as input and generated by the generator.
        """
        return self._input_token_count + self._output_token_count


    @property
    def request_count(self):
        """ Return the total number of requests made to the generator.
        """
        return self._request_count


    def get_stats(self) -> tuple[int, int, int]:
        """ Return the current statistics for the generator.
        """
        return self._input_token_count, self._output_token_count, self._request_count


    def print_stats(self):
        """ Print the current statistics for the generator.
        """
        logger.info("Input token count: %d", self._input_token_count)
        logger.info("Output token count: %d", self._output_token_count)
        logger.info("Total token count: %d", self._input_token_count + self._output_token_count)
        logger.info("Request count: %d", self._request_count)


    def _enforce_limits(self) -> None:
        """Check if we need to enforce a rate limit and wait if so."""
        self._enforce_rpm_limit()
        self._enforce_tpm_limit()


    def _enforce_rpm_limit(self) -> None:
        """Enforce requests per minute limit."""
        if self._rpm_limit is None or len(self._recent_requests) < self._rpm_limit:
            return

        wait_time = time.time() - self._recent_requests[0][0]
        if wait_time < CACHE_RETENTION_TIME:
            logger.warning("Request limit met, waiting %d seconds...", ceil(CACHE_RETENTION_TIME - wait_time))

        while time.time() - self._recent_requests[0][0] < CACHE_RETENTION_TIME:
            time.sleep(1)
        self._recent_requests.pop(0)


    def _enforce_tpm_limit(self) -> None:
        """Enforce tokens per minute limit."""
        if self._tpm_limit is None:
            return

        while sum(out_tokens for _, out_tokens in self._recent_requests) >= self._tpm_limit:
            wait_time = time.time() - self._recent_requests[0][0]
            if wait_time < CACHE_RETENTION_TIME:
                logger.warning("Token limit met, waiting %d seconds...", ceil(CACHE_RETENTION_TIME - wait_time))

            while time.time() - self._recent_requests[0][0] < CACHE_RETENTION_TIME:
                time.sleep(1)
            self._recent_requests.pop(0)


    def generate_async(
            self,
            prompts: list[str],
            system_prompt: str | None = None,
            **kwargs
    ) -> list[GenericResponse]:
        """ Generate text using the specified backend in async mode.
        """
        import asyncio
        logger.debug("Generating %d prompts in async mode.", len(prompts))

        # Set system prompt if not provided
        if self._system_prompt is not None and system_prompt is None:
            system_prompt = self._system_prompt

        # Fall back to synchronous generation for non-async backends or single prompts
        if not self._async_mode or len(prompts) < 2 or self._backend != "vllm":
            return [self.generate(p, system_prompt, **kwargs)[0] for p in prompts]

        # Use async vLLM batch generation for multiple prompts
        batch = asyncio.run(self._generate_vllm_async(prompts, system_prompt, **kwargs))

        # Update token counts
        self._update_token_counts(batch)
        return batch


    def _check_limits(self) -> None:
        """Check if generator limits have been reached."""
        if ((self._max_input_tokens is not None and self._input_token_count > self._max_input_tokens) or
            (self._max_output_tokens is not None and self._output_token_count > self._max_output_tokens) or
            (self._max_requests is not None and self._request_count > self._max_requests)):
            raise RuntimeError("Generator limits reached.")


    def _handle_generation_error(self, error: Exception, attempt: int) -> None:
        """Handle generation errors with exponential backoff retry logic."""
        cooldown = BASE_COOLDOWN * (2 ** (attempt - 1))
        logger.warning("%s when generating response: %s — attempt %d of %d, retrying in %d seconds...",
                       type(error).__name__, error, attempt, MAX_ATTEMPTS, cooldown)
        time.sleep(cooldown)

        if attempt == MAX_ATTEMPTS:
            raise RuntimeError("Max attempts reached, unable to generate response.") from error


    def _update_token_counts(self, responses: list[GenericResponse]) -> None:
        """Update internal token and request counts."""
        in_tokens = int(sum(r.prompt_tokens for r in responses))
        out_tokens = int(sum(r.completion_tokens for r in responses))

        self._input_token_count += in_tokens
        self._output_token_count += out_tokens
        self._request_count += len(responses)


    def generate(
        self,
        prompt: str,
        system_prompt: str | None = None,
        **kwargs
    ) -> list[GenericResponse]:
        """ Generate text using the specified backend.
        """
        self._check_limits()

        # Set system prompt if not provided
        if self._system_prompt is not None and system_prompt is None:
            system_prompt = self._system_prompt

        if self._generator is None:
            raise RuntimeError(f"Generator not initialized. Possible illegal backend '{self._backend}'")

        for attempt in range(1, MAX_ATTEMPTS + 1):
            # Wait for rate limit if needed
            self._enforce_limits()

            try:
                # Generate response
                start_time = time.time()
                responses = self._generator(prompt, system_prompt, **kwargs)

                # Add request to recent requests for rate limiting
                if self._rpm_limit is not None:
                    out_tokens = int(sum(r.completion_tokens for r in responses))
                    if out_tokens > 0:
                        self._recent_requests.append((start_time, out_tokens))

                # Update token counts and return
                self._update_token_counts(responses)
                logger.debug("Generation complete: %d prompt tokens, %d completion tokens.",
                             int(sum(r.prompt_tokens for r in responses)),
                             int(sum(r.completion_tokens for r in responses)))
                return responses

            except Exception as e:
                if self._backend == "gemini":
                    import google.api_core.exceptions
                    if not isinstance(e, google.api_core.exceptions.GoogleAPIError):
                        raise
                self._handle_generation_error(e, attempt)

        raise RuntimeError("Max attempts reached, unable to generate response.")
