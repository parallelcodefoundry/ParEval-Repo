"""Naively translates a repository file-by-file from one execution model to
   another execution model using an Ollama model
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

class NaiveOllamaTranslator(NaiveTranslator):

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
        super().__init__(input_repo, output_repo, src_model, dst_model,
                         dst_config, llm_name, log_interactions, dry,
                         hide_progress)
        self._model = None

    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        completion = chat(model=self._llm_name,
                          messages=[{"role": "system", "content": system_prompt},
                                    {"role": "user", "content": prompt}],
                          stream=False,
                          options={"num_ctx": 65536})
        return completion.message.content
