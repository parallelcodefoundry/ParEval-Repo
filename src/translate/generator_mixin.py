""" Mixin class for providing a unified generative interface within other classes.

    author: Daniel Nichols
    date: November 2024
"""
import os
import time
from typing import Optional, Literal, Callable, List, Tuple, Dict

class GeneratorMixin:

    _backend: Literal["openai", "gemini", "hf", "local"]
    _llm_name: str
    _generator: Optional[Callable] = None
    _sleep_time: Optional[float] = None
    _sleep_every: Optional[int] = None
    _max_input_tokens: Optional[int] = None
    _max_output_tokens: Optional[int] = None
    _max_requests: Optional[int] = None
    _input_token_count: int = 0
    _output_token_count: int = 0
    _request_count: int = 0

    _openai_client: Optional['OpenAI'] = None  # type: ignore # noqa: F821
    _gemini_client: Optional["GenerativeAI"] = None  # type: ignore # noqa: F821
    _hf_inference_client: Optional["InferenceClient"] = None  # type: ignore # noqa: F821

    def __init__(
        self,
        backend: Literal["openai", "gemini", "hf", "local"],
        llm_name: str,
        sleep_time: Optional[float] = None,
        sleep_every: Optional[int] = None,
        max_input_tokens: Optional[int] = None,
        max_output_tokens: Optional[int] = None,
        max_requests: Optional[int] = None,
    ):
        self._backend = backend
        self._llm_name = llm_name
        self._sleep_time = sleep_time
        self._sleep_every = sleep_every
        self._max_input_tokens = max_input_tokens
        self._max_output_tokens = max_output_tokens
        self._max_requests = max_requests

        if self._backend not in ["openai", "gemini", "hf", "local"]:
            raise ValueError("Invalid backend specified.")

        if self._backend == "openai":
            from openai import OpenAI
            self._openai_client = OpenAI()
            self._generator = self._generate_openai
        elif self._backend == "gemini":
            import google.generativeai as genai
            if not os.environ.get("GEMINI_API_KEY"):
                raise ValueError("GEMINI_API_KEY environment variable not set.")
            genai.configure(api_key=os.environ["GEMINI_API_KEY"])
            self._gemini_client = genai.GenerativeModel(self._llm_name)
            self._generator = self._generate_gemini
        elif self._backend == "hf":
            from huggingface_hub import InferenceClient
            if not os.environ.get("HF_API_KEY"):
                raise ValueError("HF_API_KEY environment variable not set.")
            self._hf_inference_client = InferenceClient(api_key=os.environ["HF_API_KEY"])
            self._generator = self._generate_hf
        else:
            raise NotImplementedError(f"backend '{self._backend}' not implemented.")

    def _format_messages_list(self, prompt: str, system_prompt: Optional[str] = None) -> List[Dict[str, str]]:
        messages = []
        if system_prompt:
            messages.append({"role": "system", "content": system_prompt})
        messages.append({"role": "user", "content": prompt})
        return messages

    def _generate_openai(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Tuple[str, int, int]:
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

        if not completion.choices:
            raise ValueError("No completions returned from OpenAI.")
        return completion.choices[0].message.content, 0, 0

    def _generate_gemini(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Tuple[str, int, int]:
        import google.generativeai as genai
        if not self._gemini_client:
            raise ValueError("Gemini client not initialized.")

        if system_prompt is not None:
            # system prompt can only be changed at model initialization, so we would have to reinitialize
            # the model for each generation; TODO -- we can implement this in the future if necessary
            raise NotImplementedError("System prompt not supported by Gemini.")
        
        response = self._gemini_client.generate_content(
            prompt,
            generation_config=genai.types.GenerationConfig(
                candidate_count=1,
                max_output_tokens=max_new_tokens,
                temperature=temperature,
                top_p=top_p,
            )
        )
        return response.text, 0, 0


    def _generate_hf(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Tuple[str, int, int]:
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

        return response.choices[0].message.content, 0, 0

    @property
    def input_token_count(self):
        return self._input_token_count
    
    @property
    def output_token_count(self):
        return self._output_token_count

    @property
    def token_count(self):
        return self._input_token_count + self._output_token_count

    @property
    def request_count(self):
        return self._request_count


    def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Optional[str]:

        # check if early exit cause we're over a limit
        if self._max_input_tokens is not None and self._input_token_count > self._max_input_tokens \
            or self._max_output_tokens is not None and self._output_token_count > self._max_output_tokens \
            or self._max_requests is not None and self._request_count > self._max_requests:
            return None

        # check if we need to sleep
        if self._sleep_time is not None and self._sleep_every is not None and \
            self._request_count != 0 and self._request_count % self._sleep_every == 0:
            time.sleep(self._sleep_time)

        if self._generator is None:
            raise RuntimeError(f"Generator not initialized. Possible illegal backend '{self._backend}'")
        response, in_tokens, out_tokens = self._generator(prompt, system_prompt, max_new_tokens, temperature, top_p, **kwargs)
        
        self._input_token_count += in_tokens
        self._output_token_count += out_tokens
        self._request_count += 1

        return response