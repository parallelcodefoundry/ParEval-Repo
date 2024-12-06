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

    def __init__(self, input_repo: Repo, output_repo: os.PathLike, src_model: str, dst_model: str, output_id: int, app_name: str, llm_name: str):
        super().__init__(input_repo, output_repo, src_model, dst_model, output_id, app_name, llm_name)
        self._model = None

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        completion = chat(model="llama3.2",
                          messages=[{"role": "system", "content": system_prompt},
                                    {"role": "user", "content": prompt}
                                    ],
                          stream=False)
        return completion.message.content
