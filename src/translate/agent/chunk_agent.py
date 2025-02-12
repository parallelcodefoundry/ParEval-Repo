# stl imports
from typing import List, Union
import os

# tpl imports
import clang.cindex

# local imports
from generator_mixin import GeneratorMixin

class ChunkFileAgent:
    """ ChunkFileAgent is responsible for chunking a file into smaller parts.
    """
    def __init__(self, generator: GeneratorMixin, max_tokens: int = 512,
                 interactions_path: os.PathLike = None):
        self._generator = generator
        self._max_tokens = max_tokens
        self._interactions_path = interactions_path

    def _is_too_long(self, source_code: str) -> bool:
        """ Determine if the source code is too long to process in one go.
        """
        token_count = self._estimate_tokens(source_code)
        return token_count > self._max_tokens

    def _get_chunk(self, source_code: str, start: int, end: int) -> List[str]:
        """ Get a chunk of the source code.
        """
        return source_code.splitlines()[start:end]


    def _split_on_newline(self, func_text: List[str]) -> List[int]:
        """ Get split points in the function text based on its length. First,
            try to find split points that fall on empty lines. If none are found
            that create chunks of less than max_tokens, split the function naively
            into chunks of max_tokens.
        """
        split_points = [0]
        prev_newline = -1
        for i in range(1, len(func_text)):
            if func_text[i] == "":
                if self._estimate_tokens(func_text[split_points[-1]:i]) < self._max_tokens:
                    prev_newline = i
                elif prev_newline != -1:
                    split_points.append(prev_newline)
                    prev_newline = -1

        if self._estimate_tokens(func_text[split_points[-1]:]) > self._max_tokens:
            # if the last or only chunk is too long, split it naively
            prev_line = split_points[-1]
            for i in range(split_points[-1], len(func_text)):
                if self._estimate_tokens(func_text[split_points[-1]:i]) > self._max_tokens:
                    split_points.append(prev_line)
                prev_line = i

        # add the end of the function as a split point and remove beginning
        split_points.append(len(func_text))
        split_points = split_points[1:]

        if len(split_points) == 1:
            raise ValueError("Function split failed.")

        return split_points


    def _get_ast_local_functions(self, filename: os.PathLike,
                                 node: clang.cindex.Cursor,
                                 local_functions: List[clang.cindex.Cursor]) -> None:
        """ Get local functions from the AST recursively.
        """
        if node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
            extent = node.extent
            if extent.start.file.name == filename:
                local_functions.append(node)
        for child in node.get_children():
            self._get_ast_local_functions(filename, child, local_functions)


    def _get_split_points(self, source_code: str, fname: os.PathLike) -> List[int]:
        """ Get split points in the source code based on function declarations.
        """
        local_functions = []
        idx = clang.cindex.Index.create()
        tu = idx.parse(fname)
        self._get_ast_local_functions(source_code, tu.cursor, local_functions)
        print(f"Local functions in {fname}:")
        for f in local_functions:
            print(f"  {f.spelling}, last line {f.extent.end.line}")

        split_points = [0]
        for func in local_functions:
            curr_line = func.extent.end.line + 1
            func_text = self._get_chunk(source_code, split_points[-1], curr_line)
            if self._estimate_tokens(func_text) > self._max_tokens:
                # if the function is too long, split it into smaller parts
                split_points.extend([func.extent.start.line + n \
                                     for n in self._split_on_newline(func_text)])
            split_points.append(func.extent.end.line)

        if len(split_points) == 1:
            print("No local function split points found")
            # if no split points were found, split the file naively
            split_points.extend(self._split_on_newline(source_code.splitlines()))

        # add the end of the file as a split point
        split_points.append(len(source_code))

        for i in range(len(split_points) - 1):
            print(f"Chunk {i}: {split_points[i]} - {split_points[i + 1]}")
            #print(f"Line text: {source_code[split_points[i]:split_points[i + 1]]}")

        if len(split_points) == 1:
            raise ValueError("File split failed.")

        return split_points


    def _points_to_chunks(self, source_code: str, split_points: List[int]) -> List[str]:
        """ Convert split points to chunks of source code.
        """
        chunks = []
        for i in range(len(split_points) - 1):
            chunks.append(source_code[split_points[i]:split_points[i + 1]])
        return chunks

    def _estimate_tokens(self, text: Union[str, List[str]]) -> int:
        """ Estimate the number of tokens in the given text.
        """
        if isinstance(text, list):
            text = "\n".join(text)
        return len(text.split())

    def chunk_file(self, source_code: str, fname: os.PathLike) -> List[str]:
        """ Process the source code file and return its chunks.
        """
        if self._is_too_long(source_code):
            print("Chunking file...")
            return self._points_to_chunks(source_code,
                                          self._get_split_points(source_code, fname))
        return [source_code]
