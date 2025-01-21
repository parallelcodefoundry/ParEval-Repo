"""Naively translates a repository file-by-file from one execution model to
   another execution model using Gemini
"""
# std imports
import os
import sys

# local imports
sys.path.append(os.path.dirname(__file__))
from naive_translator import NaiveTranslator
from repo import Repo


class NaiveGeminiTranslator(NaiveTranslator):

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
        import google.generativeai as genai
        genai.configure(api_key=os.environ["GEMINI_API_KEY"])
        self._model = genai.GenerativeModel("gemini-1.5-flash")

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        return self._model.generate_content(prompt).text
