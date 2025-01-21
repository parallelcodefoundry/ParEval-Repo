"""Naively translates a repository file-by-file from one execution model to
   another execution model using an locally-hosted vLLM model
"""
# std imports
import os
import sys

# tpl imports
from openai import OpenAI

# local imports
sys.path.append(os.path.dirname(__file__))
from naive_translator import NaiveTranslator
from repo import Repo

class NaiveVLLMTranslator(NaiveTranslator):

    def __init__(
            self,
            input_repo: Repo,
            output_repo: os.PathLike,
            src_model: str,
            dst_model: str,
            dst_config: dict,
            llm_name: str,
            log_interactions: bool = False,
            dry: bool = False,
            hide_progress: bool = False
    ):
        super().__init__(input_repo=input_repo, output_repo=output_repo,
                         src_model=src_model, dst_model=dst_model,
                         dst_config=dst_config, llm_name=llm_name,
                         log_interactions=log_interactions, dry=dry,
                         hide_progress=hide_progress)
        self._model = OpenAI(
            base_url="http://localhost:8000/v1",
            api_key="token-abc123"
        )

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        if self._llm_name == 'llama3.3':
            self._llm_name = 'meta-llama/Llama-3.3-70B-Instruct'
        elif self._llm_name == 'llama3.2':
            self._llm_name = 'meta-llama/Llama-3.2-3B-Instruct'
        elif self._llm_name == 'llama3.1':
            self._llm_name = 'meta-llama/Llama-3.1-8B-Instruct'
        completion = self._model.chat.completions.create(
            model=self._llm_name,
            messages=[{"role": "system", "content": system_prompt},
                      {"role": "user", "content": prompt}],
            stream=False,
            temperature=0.2,
            top_p=0.96,
            n=1
        )
        return completion.choices[0].message.content
