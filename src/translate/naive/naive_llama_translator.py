"""Naively translates a repository file-by-file from one execution model to
   another execution model using an OpenAI model
"""
# std imports
import os
import sys

# tpl imports
from ollama import chat
from ollama import ChatResponse

# local imports
sys.path.append(os.path.dirname(__file__))
from naive_translator import NaiveTranslator
from repo import Repo

class NaiveLlamaTranslator(NaiveTranslator):

    def __init__(self, input_repo: Repo, output_repo: os.PathLike, src_model: str, dst_model: str, output_id: int, app_name: str, llm_name: str, dst_config: os.PathLike):
        super().__init__(input_repo, output_repo, src_model, dst_model, output_id, app_name, llm_name, dst_config)
        self._model = None

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        if self._llm_name.lower() == "llama-3.3".lower():
            self._llm_name = "llama3.3"
        if self._llm_name.lower() == "llama-3.2".lower():
            self._llm_name = "llama3.2"
        elif self._llm_name.lower() == "llama-3.1".lower():
            self._llm_name = "llama3.1"
        completion = chat(model=self._llm_name,
                          messages=[{"role": "system", "content": system_prompt},
                                    {"role": "user", "content": prompt}
                                    ],
                          stream=False,
                          options={"num_ctx": 65536})
        return completion.message.content
