""" Mixin class for providing a unified generative interface within other classes.

    author: Daniel Nichols
    date: November 2024
"""
import os
import time
import pickle
import atexit
from typing import Optional, Literal, Callable, List, Tuple, Dict, Union
from math import ceil

class GenericResponse:
    """ Class to hold a generic response from any generator.
    """

    response: str
    reasoning: Optional[str] = None
    prompt_tokens: float
    completion_tokens: float

    def __init__(self, response: str,
                 prompt_tokens: float,
                 completion_tokens: float,
                 reasoning: Optional[str] = None):
        self.response = response
        self.reasoning = reasoning
        self.prompt_tokens = prompt_tokens
        self.completion_tokens = completion_tokens


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

    MAX_ATTEMPTS = 3

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
        self._rpm_limit = rpm_limit
        self._tpm_limit = tpm_limit
        self._max_input_tokens = max_input_tokens
        self._max_output_tokens = max_output_tokens
        self._max_requests = max_requests
        self._system_prompt = system_prompt
        self._disable_request_cache = disable_request_cache
        self._async_mode = async_mode

        if self._backend not in ["openai", "gemini", "hf", "vllm", "local"]:
            raise ValueError(f"Invalid backend specified: '{self._backend}'")

        if self._backend == "openai":
            from openai import OpenAI
            self._openai_client = OpenAI()
            self._generator = self._generate_openai
            if self._rpm_limit is None:
                self._rpm_limit = 100
            if self._tpm_limit is None:
                self._tpm_limit = 150000

        elif self._backend == "vllm":
            # Using OpenAI server API
            if self._async_mode:
                from openai import AsyncOpenAI as OpenAI
            else:
                from openai import OpenAI
            self._vllm_client = OpenAI(
                base_url="http://127.0.0.1:8000/v1",
                api_key="token_abc123",
                timeout=1200
            )
            self._generator = self._generate_vllm

        elif self._backend == "gemini":
            import google.generativeai as genai
            if not os.environ.get("GEMINI_API_KEY"):
                raise ValueError("GEMINI_API_KEY environment variable not set.")
            genai.configure(api_key=os.environ["GEMINI_API_KEY"])
            self._gemini_client = genai.GenerativeModel(self._llm_name,
                                                        system_instruction=self._system_prompt)
            self._generator = self._generate_gemini
            if self._rpm_limit is None:
                self._rpm_limit = 7
            if self._tpm_limit is None:
                self._tpm_limit = 6000

        elif self._backend == "hf":
            from huggingface_hub import InferenceClient
            if not os.environ.get("HF_API_KEY"):
                raise ValueError("HF_API_KEY environment variable not set.")
            self._hf_inference_client = InferenceClient(api_key=os.environ["HF_API_KEY"])
            self._generator = self._generate_hf

        else:
            raise NotImplementedError(f"backend '{self._backend}' not implemented.")

        if self._rpm_limit is not None or self._tpm_limit is not None:
            self._recent_requests = []

            # load recent requests from cache
            if not disable_request_cache:
                try:
                    with open(f".request_cache_{self._backend}.pkl", "rb") as f:
                        self._recent_requests = pickle.load(f)
                        print(f"Loaded {len(self._recent_requests)} recent requests from cache.")
                except FileNotFoundError:
                    pass

                atexit.register(self._cleanup)


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

        is_reasoning = False
        text = self._format_messages_list(prompt, system_prompt)
        if "QwQ" in self._llm_name or "qwq" in self._llm_name:
            if system_prompt is not None:
                # Merge system prompt into main prompt if using reasoning model
                text = self._format_messages_list(system_prompt + "\n" + prompt, None)
            # Adjust temp and top_p
            temperature = 0.6
            top_p = 0.95
            max_new_tokens = 8192
            is_reasoning = True

        completion = self._vllm_client.chat.completions.create(
            model=self._llm_name,
            messages=text,
            temperature=temperature,
            top_p=top_p,
            max_tokens=max_new_tokens,
            n=n,
            **kwargs
        )

        if not (completion and completion.choices):
            raise ValueError("No completions returned from vLLM.")
        return [GenericResponse(c.message.content if c.message.content else "",
                                completion.usage.prompt_tokens // n if completion.usage else 0,
                                completion.usage.completion_tokens // n if completion.usage else 0,
                                c.message.reasoning_content if is_reasoning else None
                                ) for c in completion.choices]

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

        is_reasoning = False
        if "QwQ" in self._llm_name or "qwq" in self._llm_name:
            # Adjust temp and top_p
            temperature = 0.6
            top_p = 0.95
            max_new_tokens = 8192
            is_reasoning = True

        if system_prompt is not None:
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
                c_split = response.split("</think>")
                if len(c_split) > 1:
                    response, reasoning = c_split[0], c_split[1]
                else:
                    reasoning = response
            results.append(GenericResponse(response,
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


    def _enforce_limits(self):
        """ Check if we need to enforce a rate limit and wait if so.
        """
        if self._rpm_limit is not None:
            if len(self._recent_requests) >= self._rpm_limit:
                wait_time = time.time() - self._recent_requests[0][0]
                if wait_time < 60:
                    print(f"Request limit met, waiting {ceil(60 - wait_time)} seconds...")
                while time.time() - self._recent_requests[0][0] < 60:
                    time.sleep(1)
                self._recent_requests.pop(0)
        if self._tpm_limit is not None:
            while sum(out_tokens for _, out_tokens in self._recent_requests) >= self._tpm_limit:
                wait_time = time.time() - self._recent_requests[0][0]
                if wait_time < 60:
                    print(f"Token limit met, waiting {ceil(60 - wait_time)} seconds...")
                while time.time() - self._recent_requests[0][0] < 60:
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

        # set system prompt if not provided
        if self._system_prompt is not None and system_prompt is None:
            system_prompt = self._system_prompt

        if not self._async_mode or len(prompts) < 2 or self._backend != "vllm":
            return [self.generate(p,
                                  system_prompt,
                                  max_new_tokens,
                                  temperature,
                                  top_p,
                                  **kwargs)[0] for p in prompts]
        batch = asyncio.run(self._generate_vllm_async(prompts,
                                                      system_prompt,
                                                      max_new_tokens,
                                                      temperature,
                                                      top_p,
                                                      **kwargs))

        # update token counts
        in_tokens = int(sum(r.prompt_tokens for r in batch))
        out_tokens = int(sum(r.completion_tokens for r in batch))
        self._input_token_count += in_tokens
        self._output_token_count += out_tokens
        self._request_count += len(batch)

        return batch


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
        import google.api_core.exceptions

        # check if early exit cause we're over a limit
        if self._max_input_tokens is not None and self._input_token_count > self._max_input_tokens \
            or self._max_output_tokens is not None and self._output_token_count > self._max_output_tokens \
            or self._max_requests is not None and self._request_count > self._max_requests:
            raise RuntimeError("Generator limits reached.")

        # set system prompt if not provided
        if self._system_prompt is not None and system_prompt is None:
            system_prompt = self._system_prompt

        if self._generator is None:
            raise RuntimeError(f"Generator not initialized. Possible illegal backend '{self._backend}'")
        num_attempts = 0
        in_tokens, out_tokens = 0, 0
        while num_attempts < self.MAX_ATTEMPTS:
            # wait for rate limit if needed
            self._enforce_limits()

            try:
                # generate response
                start_time = time.time()
                responses = self._generator(prompt,
                                            system_prompt,
                                            max_new_tokens,
                                            temperature,
                                            top_p,
                                            n,
                                            **kwargs)
                in_tokens = int(sum(r.prompt_tokens for r in responses))
                out_tokens = int(sum(r.completion_tokens for r in responses))
            except google.api_core.exceptions.GoogleAPIError as e:
                # failure, wait 15 sec. and retry
                num_attempts += 1
                print(f"{type(e)} when generating response:")
                print(e)
                print(f"Attempt {num_attempts} of {self.MAX_ATTEMPTS}, retrying in 15 seconds...")
                time.sleep(15)
                if num_attempts == self.MAX_ATTEMPTS:
                    raise RuntimeError("Max attempts reached, unable to generate response.") \
                        from e
            else:
                # success, stop trying
                break
            finally:
                # add request to recent requests
                if self._rpm_limit is not None and out_tokens > 0:
                    self._recent_requests.append((start_time, out_tokens))

        # update token counts
        self._input_token_count += in_tokens
        self._output_token_count += out_tokens
        self._request_count += n

        return responses
