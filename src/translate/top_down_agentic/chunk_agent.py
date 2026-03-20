"""ChunkFileAgent is responsible for chunking files into smaller parts for processing.

This agent handles the splitting of large source code files into manageable chunks
that can be processed by LLMs within token limits.
"""

import logging
from typing import List, Union, Optional
import os
from math import ceil

logger = logging.getLogger("pareval-repo")

# Third-party imports
from langchain_text_splitters import Language
from langchain_text_splitters import RecursiveCharacterTextSplitter as rcts

# Local imports
from generator_mixin import GeneratorMixin
from .utils import estimate_tokens


class ChunkFileAgent:
    """Agent responsible for chunking files into smaller parts for LLM processing."""

    # Average characters per token for token estimation
    AVG_CHAR_PER_TOKEN = 3.5

    def __init__(self,
                 generator: GeneratorMixin,
                 max_tokens: int = 1024,
                 interactions_paths: Optional[List[os.PathLike]] = None,
                 language: str = 'c'):
        """Initialize the ChunkFileAgent.

        Args:
            generator: The generator mixin for LLM interactions
            max_tokens: Maximum tokens per chunk
            interactions_paths: Optional paths for logging interactions
            language: Programming language for text splitting
        """
        self._generator = generator
        self._max_tokens = max_tokens
        self._interactions_paths = interactions_paths
        self._language = Language(language)
        self._splitter = self._create_splitter()


    def _create_splitter(self) -> rcts:
        """Create and configure the text splitter."""
        chunk_size = ceil(self._max_tokens * self.AVG_CHAR_PER_TOKEN)
        return rcts.from_language(
            self._language,
            chunk_size=chunk_size,
            chunk_overlap=0
        )


    def _is_too_long(self, source_code: str) -> bool:
        """Determine if the source code exceeds the maximum token limit."""
        token_count = estimate_tokens(source_code, self.AVG_CHAR_PER_TOKEN)
        return token_count > self._max_tokens


    def chunk_file(self, source_code: str) -> List[str]:
        """Process the source code file and return its chunks.

        Args:
            source_code: The source code to chunk

        Returns:
            List of code chunks, or the original code if no chunking needed
        """
        if not source_code.strip():
            return [""]

        if self._is_too_long(source_code):
            logger.debug("File exceeds token limit; chunking...")
            try:
                docs = self._splitter.create_documents([source_code])
                chunks = [doc.page_content for doc in docs]
                logger.debug("Split into %d chunks.", len(chunks))
                return chunks
            except Exception as e:
                logger.warning("Error chunking file: %s", e)
                # Fallback: return original code if chunking fails
                return [source_code]

        return [source_code]
