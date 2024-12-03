"""Naively translates a repository file-by-file from one execution model to
   another execution model using an OpenAI model
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

class NaiveOpenAITranslator(NaiveTranslator):

    def __init__(self, input_repo: Repo, output_repo: os.PathLike, src_model: str, dst_model: str, output_id: int, app_name: str, llm_name: str):
        super().__init__(input_repo, output_repo, src_model, dst_model, output_id, app_name, llm_name)
        self._model = OpenAI()

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        if self._llm_name == "gpt-3.5":
            model = "gpt-3.5-turbo"
        elif self._llm_name == "gpt-4o":
            model = "gpt-4o"
        completion = self._model.chat.completions.create(
            model=model,
            messages=[
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": prompt}
            ],
            max_tokens=4096,
            temperature=0.2,
            top_p=0.96,
            stream=False,
            n=1
        )
        return completion.choices[0].message.content
