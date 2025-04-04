# stl imports
from typing import List, Union, Optional
import os
from math import ceil

# tpl imports
from langchain_text_splitters import Language
from langchain_text_splitters import RecursiveCharacterTextSplitter as rcts

# local imports
from generator_mixin import GeneratorMixin

class ChunkFileAgent:
    """ ChunkFileAgent is responsible for chunking a file into smaller parts.
    """

    AVG_CHAR_PER_TOKEN = 3.5

    def __init__(self, generator: GeneratorMixin, max_tokens: int = 1024,
                 interactions_path: Optional[os.PathLike] = None,
                 language: str = 'c'):
        self._generator = generator
        self._max_tokens = max_tokens
        self._interactions_path = interactions_path
        self._language = Language(language)
        self._splitter = rcts.from_language(self._language,
                                            chunk_size=ceil(max_tokens * self.AVG_CHAR_PER_TOKEN),
                                            chunk_overlap=0)

    def _is_too_long(self, source_code: str) -> bool:
        """ Determine if the source code is too long to process in one go.
        """
        token_count = self._estimate_tokens(source_code)
        return token_count > self._max_tokens

    def _estimate_tokens(self, text: Union[str, List[str]]) -> int:
        """ Estimate the number of tokens in the given text.
        """
        return ceil(len(text) * self.AVG_CHAR_PER_TOKEN)

    def chunk_file(self, source_code: str) -> List[str]:
        """ Process the source code file and return its chunks.
        """
        if self._is_too_long(source_code):
            print("Chunking file...")
            docs = self._splitter.create_documents([source_code])
            return [doc.page_content for doc in docs]
        return [source_code]
