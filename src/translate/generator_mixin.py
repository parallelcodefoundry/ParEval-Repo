""" Mixin class for providing a unified generative interface within other classes.

    author: Daniel Nichols
    date: November 2024
"""
import os
import time
import pickle
import atexit
from typing import Optional, Literal, Callable, List, Tuple, Dict, Union, Any
from math import ceil
from dataclasses import dataclass

@dataclass
class GenericResponse:
    """ Class to hold a generic response from any generator.
    """
    response: str
    prompt_tokens: float
    completion_tokens: float
    reasoning: Optional[str] = None


# Constants
MAX_ATTEMPTS = 3
DEFAULT_OPENAI_RPM = 100
DEFAULT_OPENAI_TPM = 150000
DEFAULT_GEMINI_RPM = 7
DEFAULT_GEMINI_TPM = 6000
VLLM_BASE_URL = os.environ.get("OPENAI_API_BASE", "http://127.0.0.1:8000/v1")
VLLM_API_KEY = "token_abc123"
VLLM_TIMEOUT = 1200
REASONING_TEMP = 0.6
REASONING_TOP_P = 0.95
REASONING_MAX_TOKENS = 8192
RETRY_DELAY = 15
CACHE_RETENTION_TIME = 60


@dataclass
class BackendConfig:
    """Configuration for different backends."""
    rpm_limit: Optional[int] = None
    tpm_limit: Optional[int] = None
    requires_api_key: bool = False
    api_key_env_var: Optional[str] = None


class GeneratorMixin:
    """ Mixin class for providing a unified generative interface within other classes.
    """

    _backend: Literal["openai", "gemini", "hf", "vllm", "local"]
    _llm_name: str
    _generator: Optional[Callable] = None
    _rpm_limit: Optional[int] = None
    _tpm_limit: Optional[int] = None
    _recent_requests: Optional[List[Tuple[int,int]]] = None
    _max_input_tokens: Optional[int] = None
    _max_output_tokens: Optional[int] = None
    _max_requests: Optional[int] = None
    _input_token_count: int = 0
    _output_token_count: int = 0
    _request_count: int = 0
    _system_prompt: Optional[str] = None
    _disable_request_cache: Optional[bool] = False

    _openai_client: Optional['OpenAI'] = None  # type: ignore # noqa: F821
    _gemini_client: Optional["GenerativeAI"] = None  # type: ignore # noqa: F821
    _hf_inference_client: Optional["InferenceClient"] = None  # type: ignore # noqa: F821
    _vllm_client: Optional['OpenAI'] = None  # type: ignore # noqa: F821
    _async_mode: bool = False

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
        rpm_limit: Optional[int] = None,
        tpm_limit: Optional[int] = None,
        max_input_tokens: Optional[int] = None,
        max_output_tokens: Optional[int] = None,
        max_requests: Optional[int] = None,
        system_prompt: Optional[str] = None,
        disable_request_cache: Optional[bool] = False,
        async_mode: bool = False,
    ):
        self._backend = backend
        self._llm_name = llm_name
        self._max_input_tokens = max_input_tokens
        self._max_output_tokens = max_output_tokens
        self._max_requests = max_requests
        self._system_prompt = system_prompt
        self._disable_request_cache = disable_request_cache
        self._async_mode = async_mode

        self._validate_backend()
        self._configure_backend(rpm_limit, tpm_limit)
        self._setup_rate_limiting()


    def _validate_backend(self) -> None:
        """Validate that the backend is supported."""
        if self._backend not in self._BACKEND_CONFIGS:
            raise ValueError(f"Invalid backend specified: '{self._backend}'")


    def _configure_backend(self, rpm_limit: Optional[int], tpm_limit: Optional[int]) -> None:
        """Configure the backend client and generator."""
        config = self._BACKEND_CONFIGS[self._backend]

        # Set rate limits from config if not provided
        self._rpm_limit = rpm_limit or config.rpm_limit
        self._tpm_limit = tpm_limit or config.tpm_limit

        # Check API key requirements
        if config.requires_api_key and config.api_key_env_var:
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


    def _setup_openai(self) -> None:
        """Setup OpenAI client and generator."""
        from openai import OpenAI
        self._openai_client = OpenAI()
        self._generator = self._generate_openai


    def _setup_vllm(self) -> None:
        """Setup vLLM client and generator."""
        if self._async_mode:
            from openai import AsyncOpenAI as OpenAI
        else:
            from openai import OpenAI
        self._vllm_client = OpenAI(
            base_url=VLLM_BASE_URL,
            api_key=VLLM_API_KEY,
            timeout=VLLM_TIMEOUT
        )
        self._generator = self._generate_vllm


    def _setup_gemini(self) -> None:
        """Setup Gemini client and generator."""
        import google.generativeai as genai
        genai.configure(api_key=os.environ["GEMINI_API_KEY"])
        self._gemini_client = genai.GenerativeModel(
            self._llm_name,
            system_instruction=self._system_prompt
        )
        self._generator = self._generate_gemini


    def _setup_huggingface(self) -> None:
        """Setup Hugging Face client and generator."""
        from huggingface_hub import InferenceClient
        self._hf_inference_client = InferenceClient(api_key=os.environ["HF_API_KEY"])
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
                print(f"Loaded {len(self._recent_requests)} recent requests from cache.")
        except FileNotFoundError:
            pass


    def _cleanup(self):
        """ Save recent requests to cache on deletion.
        """
        if self._recent_requests and not self._disable_request_cache:
            if time.time() - self._recent_requests[-1][0] <= 60:
                with open(f".request_cache_{self._backend}.pkl", "wb") as f:
                    pickle.dump(self._recent_requests, f)


    def _format_messages_list(self, prompt: str,
                              system_prompt: Union[str, None]) \
                              -> List[Dict[str, str]]:
        """ Format the prompt and system prompt into a list of messages for OpenAI.
        """
        messages = []
        if system_prompt is not None:
            messages.append({"role": "system", "content": system_prompt})
        messages.append({"role": "user", "content": prompt})
        return messages


    def _generate_openai(
            self,
            prompt: str,
            system_prompt: str,
            max_new_tokens: int = 2048,
            temperature: Optional[float] = None,
            top_p: Optional[float] = None,
            n: int = 1,
            **kwargs
    ) -> List[GenericResponse]:
        """ Generate text using the OpenAI API.
        """
        if not self._openai_client:
            raise ValueError("OpenAI client not initialized.")

        completion = self._openai_client.chat.completions.create(
            model=self._llm_name,
            messages=self._format_messages_list(prompt, system_prompt),
            max_tokens=max_new_tokens,
            temperature=temperature,
            top_p=top_p,
            n=1,
            **kwargs
        )

        if not (completion and completion.choices):
            raise ValueError("No completions returned from OpenAI.")

        return [GenericResponse(c.message.content,
                                completion.usage.prompt_tokens // n,
                                completion.usage.completion_tokens // n
                                ) for c in completion.choices]


    def _is_reasoning_model(self) -> bool:
        """Check if the current model is a reasoning model."""
        return "QwQ" in self._llm_name or "qwq" in self._llm_name


    def _adjust_parameters_for_reasoning(self, temperature: Optional[float],
                                       top_p: Optional[float],
                                       max_new_tokens: int) -> Tuple[float, float, int]:
        """Adjust parameters for reasoning models."""
        if self._is_reasoning_model():
            return REASONING_TEMP, REASONING_TOP_P, REASONING_MAX_TOKENS
        return temperature, top_p, max_new_tokens


    def _prepare_messages_for_reasoning(self, prompt: str, system_prompt: Optional[str]) -> List[Dict[str, str]]:
        """Prepare messages for reasoning models by merging system prompt."""
        if self._is_reasoning_model() and system_prompt is not None:
            return self._format_messages_list(system_prompt + "\n" + prompt, None)
        return self._format_messages_list(prompt, system_prompt)


    def _generate_vllm(
            self,
            prompt: str,
            system_prompt: str,
            max_new_tokens: int = 2048,
            temperature: Optional[float] = None,
            top_p: Optional[float] = None,
            n: int = 1,
            **kwargs
    ) -> List[GenericResponse]:
        """ Generate text using the vLLM via the OpenAI server API.
        """
        if not self._vllm_client:
            raise ValueError("vLLM (OpenAI) client not initialized.")

        # Adjust parameters for reasoning models
        temperature, top_p, max_new_tokens = self._adjust_parameters_for_reasoning(
            temperature, top_p, max_new_tokens
        )

        # Prepare messages
        messages = self._prepare_messages_for_reasoning(prompt, system_prompt)
        is_reasoning = self._is_reasoning_model()

        completion = self._vllm_client.chat.completions.create(
            model=self._llm_name,
            messages=messages,
            temperature=temperature,
            top_p=top_p,
            max_tokens=max_new_tokens,
            n=n,
            **kwargs
        )

        if not (completion and completion.choices):
            raise ValueError("No completions returned from vLLM.")

        return [GenericResponse(
            c.message.content if c.message.content else "",
            completion.usage.prompt_tokens // n if completion.usage else 0,
            completion.usage.completion_tokens // n if completion.usage else 0,
            c.message.reasoning_content if is_reasoning else None
        ) for c in completion.choices]


    def _parse_reasoning_response(self, response: str) -> Tuple[str, Optional[str]]:
        """Parse reasoning response to extract reasoning and content."""
        if "</think>" in response:
            parts = response.split("</think>")
            return parts[1] if len(parts) > 1 else "", parts[0]
        return response, response


    async def _generate_vllm_async(
            self,
            prompts: List[str],
            system_prompt: str,
            max_new_tokens: int = 2048,
            temperature: Optional[float] = None,
            top_p: Optional[float] = None,
            **kwargs
    ) -> List[GenericResponse]:
        """ Generate text using the vLLM via the OpenAI server API in async mode.
        """
        if not self._vllm_client:
            raise ValueError("vLLM (OpenAI) client not initialized.")

        # Adjust parameters for reasoning models
        temperature, top_p, max_new_tokens = self._adjust_parameters_for_reasoning(
            temperature, top_p, max_new_tokens
        )
        is_reasoning = self._is_reasoning_model()

        # Prepare prompts for reasoning models
        if is_reasoning and system_prompt is not None:
            prompts = [system_prompt + "\n" + p for p in prompts]

        completion = await self._vllm_client.completions.create(
            model=self._llm_name,
            prompt=prompts,
            temperature=temperature,
            top_p=top_p,
            max_tokens=max_new_tokens,
            **kwargs
        )

        if not (completion and completion.choices):
            raise ValueError("No completions returned from vLLM.")

        results = []
        for c in completion.choices:
            response, reasoning = c.text, None
            print(f"Raw response: {response}")

            if is_reasoning:
                response, reasoning = self._parse_reasoning_response(response)

            results.append(GenericResponse(
                response,
                completion.usage.prompt_tokens // len(prompts),
                completion.usage.completion_tokens // len(prompts),
                reasoning
            ))

        return results


    def _generate_gemini(
            self,
            prompt: str,
            system_prompt: str,
            max_new_tokens: int = 2048,
            temperature: Optional[float] = None,
            top_p: Optional[float] = None,
            n: int = 1,
            **kwargs
    ) -> List[GenericResponse]:
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
            generation_config=genai.types.GenerationConfig(
                candidate_count=1,
                max_output_tokens=max_new_tokens,
                temperature=temperature,
                top_p=top_p,
            )
        )
        return [GenericResponse(response.text,
                                response.usage_metadata.prompt_token_count,
                                response.usage_metadata.candidates_token_count)]


    def _generate_hf(
        self,
        prompt: str,
        system_prompt: str,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        n: int = 1,
        **kwargs
    ) -> List[GenericResponse]:
        """ Generate text using the Hugging Face API.
        """
        if not self._hf_inference_client:
            raise ValueError("Hugging Face Inference client not initialized.")

        response = self._hf_inference_client.chat.completions.create(
            model=self._llm_name,
            messages=self._format_messages_list(prompt, system_prompt),
            max_tokens=max_new_tokens,
            temperature=temperature,
            top_p=top_p,
            n=1,
            **kwargs
        )

        if not response.choices:
            raise ValueError("No completions returned from Hugging Face.")

        return [GenericResponse(response.choices[0].message.content,
                                response.usage.prompt_tokens,
                                response.usage.completion_tokens)]


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


    def get_stats(self) -> Tuple[int, int, int]:
        """ Return the current statistics for the generator.
        """
        return self._input_token_count, self._output_token_count, self._request_count


    def print_stats(self):
        """ Print the current statistics for the generator.
        """
        print(f"Input token count: {self._input_token_count}")
        print(f"Output token count: {self._output_token_count}")
        print(f"Total token count: {self._input_token_count + self._output_token_count}")
        print(f"Request count: {self._request_count}")


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
            print(f"Request limit met, waiting {ceil(CACHE_RETENTION_TIME - wait_time)} seconds...")

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
                print(f"Token limit met, waiting {ceil(CACHE_RETENTION_TIME - wait_time)} seconds...")

            while time.time() - self._recent_requests[0][0] < CACHE_RETENTION_TIME:
                time.sleep(1)
            self._recent_requests.pop(0)


    def generate_async(
            self,
            prompts: List[str],
            system_prompt: Optional[str] = None,
            max_new_tokens: int = 2048,
            temperature: Optional[float] = None,
            top_p: Optional[float] = None,
            **kwargs
    ) -> List[GenericResponse]:
        """ Generate text using the specified backend in async mode.
        """
        import asyncio
        print("Generating in async mode...")

        # Set system prompt if not provided
        if self._system_prompt is not None and system_prompt is None:
            system_prompt = self._system_prompt

        # Fall back to synchronous generation for non-async backends or single prompts
        if not self._async_mode or len(prompts) < 2 or self._backend != "vllm":
            return [self.generate(p, system_prompt, max_new_tokens, temperature, top_p, **kwargs)[0]
                   for p in prompts]

        # Use async vLLM generation for multiple prompts
        batch = asyncio.run(self._generate_vllm_async(
            prompts, system_prompt, max_new_tokens, temperature, top_p, **kwargs
        ))

        # Update token counts
        self._update_token_counts(batch, len(batch))
        return batch


    def _check_limits(self) -> None:
        """Check if generator limits have been reached."""
        if ((self._max_input_tokens is not None and self._input_token_count > self._max_input_tokens) or
            (self._max_output_tokens is not None and self._output_token_count > self._max_output_tokens) or
            (self._max_requests is not None and self._request_count > self._max_requests)):
            raise RuntimeError("Generator limits reached.")


    def _handle_generation_error(self, error: Exception, attempt: int) -> None:
        """Handle generation errors with retry logic."""
        print(f"{type(error)} when generating response:")
        print(error)
        print(f"Attempt {attempt} of {MAX_ATTEMPTS}, retrying in {RETRY_DELAY} seconds...")
        time.sleep(RETRY_DELAY)

        if attempt == MAX_ATTEMPTS:
            raise RuntimeError("Max attempts reached, unable to generate response.") from error


    def _update_token_counts(self, responses: List[GenericResponse], n: int) -> None:
        """Update internal token and request counts."""
        in_tokens = int(sum(r.prompt_tokens for r in responses))
        out_tokens = int(sum(r.completion_tokens for r in responses))

        self._input_token_count += in_tokens
        self._output_token_count += out_tokens
        self._request_count += n


    def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        n: int = 1,
        **kwargs
    ) -> List[GenericResponse]:
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
                responses = self._generator(
                    prompt, system_prompt, max_new_tokens, temperature, top_p, n, **kwargs
                )

                # Add request to recent requests for rate limiting
                if self._rpm_limit is not None:
                    out_tokens = int(sum(r.completion_tokens for r in responses))
                    if out_tokens > 0:
                        self._recent_requests.append((start_time, out_tokens))

                # Update token counts and return
                self._update_token_counts(responses, n)
                return responses

            except Exception as e:
                if self._backend == "gemini":
                    import google.api_core.exceptions
                    if not isinstance(e, google.api_core.exceptions.GoogleAPIError):
                        raise
                self._handle_generation_error(e, attempt)
