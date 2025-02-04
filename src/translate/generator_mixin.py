""" Mixin class for providing a unified generative interface within other classes.

    author: Daniel Nichols
    date: November 2024
"""
import os
import time
from typing import Optional, Literal, Callable, List, Tuple, Dict

class GeneratorMixin:
    """ Mixin class for providing a unified generative interface within other classes.
    """

    _backend: Literal["openai", "gemini", "hf", "local"]
    _llm_name: str
    _generator: Optional[Callable] = None
    _rpm_limit: Optional[int] = None
    _recent_requests: Optional[List] = None
    _max_input_tokens: Optional[int] = None
    _max_output_tokens: Optional[int] = None
    _max_requests: Optional[int] = None
    _input_token_count: int = 0
    _output_token_count: int = 0
    _request_count: int = 0
    _system_prompt: Optional[str] = None

    _openai_client: Optional['OpenAI'] = None  # type: ignore # noqa: F821
    _gemini_client: Optional["GenerativeAI"] = None  # type: ignore # noqa: F821
    _hf_inference_client: Optional["InferenceClient"] = None  # type: ignore # noqa: F821

    def __init__(
        self,
        backend: Literal["openai", "gemini", "hf", "local"],
        llm_name: str,
        rpm_limit: Optional[int] = None,
        max_input_tokens: Optional[int] = None,
        max_output_tokens: Optional[int] = None,
        max_requests: Optional[int] = None,
        system_prompt: Optional[str] = None
    ):
        self._backend = backend
        self._llm_name = llm_name
        self._rpm_limit = rpm_limit
        self._max_input_tokens = max_input_tokens
        self._max_output_tokens = max_output_tokens
        self._max_requests = max_requests
        self._system_prompt = system_prompt

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
            self._gemini_client = genai.GenerativeModel(self._llm_name,
                                                        system_instruction=self._system_prompt)
            self._generator = self._generate_gemini
            if self._rpm_limit is None:
                self._rpm_limit = 15
        elif self._backend == "hf":
            from huggingface_hub import InferenceClient
            if not os.environ.get("HF_API_KEY"):
                raise ValueError("HF_API_KEY environment variable not set.")
            self._hf_inference_client = InferenceClient(api_key=os.environ["HF_API_KEY"])
            self._generator = self._generate_hf
        else:
            raise NotImplementedError(f"backend '{self._backend}' not implemented.")

        if self._rpm_limit is not None:
            self._recent_requests = []


    def _format_messages_list(self, prompt: str,
                              system_prompt: Optional[str] = None) \
                              -> List[Dict[str, str]]:
        """ Format the prompt and system prompt into a list of messages for OpenAI.
        """
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

        if not completion.choices:
            raise ValueError("No completions returned from OpenAI.")
        return completion.choices[0].message.content, \
            completion.usage.prompt_tokens, completion.usage.completion_tokens

    def _generate_gemini(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Tuple[str, int, int]:
        """ Generate text using the Gemini API.
        """
        import google.generativeai as genai
        if not self._gemini_client:
            raise ValueError("Gemini client not initialized.")

        if system_prompt is not None and system_prompt != self._system_prompt:
            # system prompt can only be changed at model initialization, so we would have to reinitialize
            # the model for each generation; TODO -- we can implement this in the future if necessary
            raise NotImplementedError("System prompt can only be changed at initialized with Gemini.")

        response = self._gemini_client.generate_content(
            prompt,
            generation_config=genai.types.GenerationConfig(
                candidate_count=1,
                max_output_tokens=max_new_tokens,
                temperature=temperature,
                top_p=top_p,
            )
        )
        return response.text, \
            response.usage_metadata.prompt_token_count, \
            response.usage_metadata.candidates_token_count


    def _generate_hf(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Tuple[str, int, int]:
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

        return response.choices[0].message.content, 0, 0

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


    def print_stats(self):
        """ Print the current statistics for the generator.
        """
        print(f"Input token count: {self._input_token_count}")
        print(f"Output token count: {self._output_token_count}")
        print(f"Total token count: {self._input_token_count + self._output_token_count}")
        print(f"Request count: {self._request_count}")


    def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        max_new_tokens: int = 2048,
        temperature: Optional[float] = None,
        top_p: Optional[float] = None,
        **kwargs
    ) -> Optional[str]:
        """ Generate text using the specified backend.
        """
        # check if early exit cause we're over a limit
        if self._max_input_tokens is not None and self._input_token_count > self._max_input_tokens \
            or self._max_output_tokens is not None and self._output_token_count > self._max_output_tokens \
            or self._max_requests is not None and self._request_count > self._max_requests:
            return None

        # check if we need to enforce a rate limit
        if self._rpm_limit is not None:
            if len(self._recent_requests) >= self._rpm_limit:
                while time.time() - self._recent_requests[0] < 60:
                    print("Rate limit met, waiting...")
                    time.sleep(1)
                self._recent_requests.pop(0)
            self._recent_requests.append(time.time())

        # set system prompt if not provided
        if self._system_prompt is not None and system_prompt is None:
            system_prompt = self._system_prompt

        if self._generator is None:
            raise RuntimeError(f"Generator not initialized. Possible illegal backend '{self._backend}'")
        response, in_tokens, out_tokens = self._generator(prompt, system_prompt, max_new_tokens, temperature, top_p, **kwargs)

        self._input_token_count += in_tokens
        self._output_token_count += out_tokens
        self._request_count += 1

        return response
