"""Naively translates a repository file-by-file from one execution model to
   another execution model using Gemini
"""
# std imports
import os
import sys

# tpl imports
import google.generativeai as genai

# local imports
sys.path.append(os.path.dirname(__file__))
from naive_translator import NaiveTranslator
from repo import Repo


class NaiveGeminiTranslator(NaiveTranslator):

    def __init__(self, input_repo: Repo, output_repo: os.PathLike, src_model: str, dst_model: str, output_id: int, app_name: str, llm_name: str, dst_config: os.PathLike):
        super().__init__(input_repo, output_repo, src_model, dst_model, output_id, app_name, llm_name, dst_config)
        genai.configure(api_key=os.environ["GEMINI_API_KEY"])
        self._model = genai.GenerativeModel("gemini-1.5-flash")

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        return self._model.generate_content(prompt).text
