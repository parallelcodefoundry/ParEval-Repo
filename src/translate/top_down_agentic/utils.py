"""Common utilities for the top_down_agentic translation agents.

This module contains shared utilities, constants, and helper functions
used across the different agent classes to reduce code duplication.
"""

import logging
import os
import re
from typing import List, Optional, Union, Dict, Any
from enum import Enum

logger = logging.getLogger("pareval-repo")


class FileType(Enum):
    """An enumeration of file types."""
    SOURCE = 1
    HEADER = 2
    BUILD = 3
    METADATA = 4
    OTHER = 5


# File extension mappings
SOURCE_EXTENSIONS = [".c", ".C", ".cc", ".cxx", ".cpp", ".c++", ".cu"]
HEADER_EXTENSIONS = [".h", ".H", ".hh", ".hpp", ".cuh"]
BUILD_EXTENSIONS = [".make"]
BUILD_NAMES = ["Makefile", "CMakeLists.txt"]
METADATA_EXTENSIONS = [".txt", ".md"]
METADATA_NAMES = ["README", "LICENSE", "INSTALL"]


def get_file_type(file_path: Union[str, os.PathLike]) -> FileType:
    """Determine the file type based on the file extension and name."""
    file_path = str(file_path)
    ext = os.path.splitext(file_path)[1]
    basename = os.path.basename(file_path)

    if ext in SOURCE_EXTENSIONS:
        return FileType.SOURCE
    elif ext in HEADER_EXTENSIONS:
        return FileType.HEADER
    elif ext in BUILD_EXTENSIONS or any(name in basename for name in BUILD_NAMES):
        return FileType.BUILD
    elif ext in METADATA_EXTENSIONS or any(name in basename for name in METADATA_NAMES):
        return FileType.METADATA
    else:
        return FileType.OTHER


def is_cpp_source_file(file_path: Union[str, os.PathLike], include_headers: bool = True) -> bool:
    """Check if a file is a C/C++ source file."""
    file_type = get_file_type(file_path)
    return file_type in [FileType.SOURCE] or (include_headers and file_type == FileType.HEADER)


def safe_get_terminal_columns() -> int:
    """Get terminal size with exception handling."""
    try:
        return os.get_terminal_size().columns
    except OSError as error:
        if error.errno == 25:
            return 80
        else:
            raise error


def extract_code_block(output: str) -> Optional[str]:
    """Extract code block from LLM output, ensuring there's only one."""
    CODE_BLOCK_PATTERN = re.compile(r"```(?:[+\w]+)?\n(.*?)\n```", re.DOTALL)
    match = CODE_BLOCK_PATTERN.search(output)
    if match is None:
        logger.warning("No code block found in output:\n%s", output)
        return None
    return match.group(1)


def ensure_directory_exists(file_path: Union[str, os.PathLike]) -> None:
    """Ensure the directory for the given file path exists."""
    directory = os.path.dirname(file_path)
    if directory:
        os.makedirs(directory, exist_ok=True)


def read_file_safely(file_path: Union[str, os.PathLike], encoding: str = "UTF-8") -> str:
    """Read file contents with proper error handling."""
    try:
        with open(file_path, 'r', encoding=encoding) as f:
            return f.read()
    except FileNotFoundError:
        return ""
    except Exception as e:
        logger.error("Error reading file %s: %s", file_path, e)
        return ""


def write_file_safely(file_path: Union[str, os.PathLike], contents: str, encoding: str = "UTF-8") -> bool:
    """Write file contents with proper error handling."""
    try:
        ensure_directory_exists(file_path)
        with open(file_path, 'w', encoding=encoding) as f:
            f.write(contents)
        return True
    except Exception as e:
        logger.error("Error writing file %s: %s", file_path, e)
        return False


def update_file_extension(file_path: Union[str, os.PathLike],
                         new_extension: str,
                         trigger_rename: Optional[Union[str, os.PathLike]] = None) -> str:
    """Update file extension, optionally with a trigger rename."""
    if trigger_rename:
        file_path = trigger_rename

    if "." in str(file_path):
        name, _ = os.path.splitext(file_path)
        return name + new_extension

    return str(file_path)


def format_dependency_list(dependencies: List[str]) -> str:
    """Format a list of dependencies as a readable string."""
    if not dependencies:
        return "None"
    return ", ".join(dependencies)


def estimate_tokens(text: Union[str, List[str]], chars_per_token: float = 3.5) -> int:
    """Estimate the number of tokens in the given text."""
    from math import ceil
    if isinstance(text, list):
        text = "".join(text)
    return ceil(len(text) / chars_per_token)
