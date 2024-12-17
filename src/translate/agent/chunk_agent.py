from typing import List, Tuple
import os
import re

class ChunkFileAgent:
    def __init__(self, max_tokens: int = 2048):
        self.max_tokens = max_tokens

    def is_too_long(self, source_code: str) -> bool:
        token_count = self._estimate_tokens(source_code)
        return token_count > self.max_tokens

    def split_into_chunks(self, source_code: str) -> List[str]:
        chunks = []
        current_chunk = ""
        token_count = 0

        for block in self._split_by_structure(source_code):
            block_tokens = self._estimate_tokens(block)
            if token_count + block_tokens > self.max_tokens:
                chunks.append(current_chunk)
                current_chunk = block
                token_count = block_tokens
            else:
                current_chunk += block
                token_count += block_tokens

        if current_chunk:
            chunks.append(current_chunk)
        return chunks

    def _split_by_structure(self, source_code: str) -> List[str]:
        pattern = r'(\n\s*\n)|(^def\s)|(^class\s)'
        split_points = [m.start() for m in re.finditer(pattern, source_code, flags=re.MULTILINE)]
        split_points.append(len(source_code))

        blocks = []
        prev_index = 0
        for index in split_points:
            blocks.append(source_code[prev_index:index])
            prev_index = index
        return blocks

    def _estimate_tokens(self, text: str) -> int:
        return len(text.split())

    def process_file(self, file_path: os.PathLike) -> List[str]:
        with open(file_path, 'r') as file:
            source_code = file.read()

        if self.is_too_long(source_code):
            return self.split_into_chunks(source_code)
        else:
            return [source_code]