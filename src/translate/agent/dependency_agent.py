""" Dependency Agent. Handles constructing a file dependency graph for a respository.

    For C/C++ uses clang -MMD to generate a dependency file for each source file. This
    is then used to construct a dependecy graph for the entire repository. Build files
    depend on the entire repository. Other metadata files such as READMEs and LICENSEs
    are disconnected components in the graph.

    author: Daniel Nichols
    date: November 2024
"""
# std imports
#from ctypes import Union
from glob import glob
import logging
import os
import subprocess
from typing import List, Optional, Union
from enum import Enum

# local imports
from generator_mixin import GeneratorMixin

class FileType(Enum):
    """ An enumeration of file types.
    """
    SOURCE = 1
    HEADER = 2
    BUILD = 3
    METADATA = 4
    OTHER = 5


class FileNode:
    """ A node in a file dependency graph.
    """

    rel_path: str
    dependencies: List['FileNode'] = []
    parents: List['FileNode'] = []
    filetype: FileType = FileType.OTHER

    def __init__(self, rel_path: str = "", filetype: FileType = FileType.OTHER):
        self.rel_path = rel_path
        if filetype:
            self.filetype = filetype
        else:
            self.filetype = self._get_filetype()

    def _get_filetype(self) -> FileType:
        """ Get the file type based on the file extension.
        """
        ext = os.path.splitext(self.rel_path)[1]
        if ext in [".c", ".C", ".cc", ".cxx", ".cpp", ".c++", ".cu"]:
            return FileType.SOURCE
        if ext in [".h", ".H", ".hh", ".hpp"]:
            return FileType.HEADER
        if ext in ["Makefile", "CMakeLists.txt"]:
            return FileType.BUILD
        if ext in ["README", "LICENSE"]:
            return FileType.METADATA
        return FileType.OTHER

    @property
    def basename(self) -> str:
        """ Get the basename of the file.
        """
        return os.path.basename(self.rel_path)


class DependencyAgent:
    """ An agent for constructing a file dependency graph for a repository.
    """

    def __init__(self, generator: GeneratorMixin, interactions_path: os.PathLike = None):
        self._generator = generator
        self._interactions_path = interactions_path

    def _get_files_by_suffixes(self, repo_path: os.PathLike,
                               suffixes: List[str]) -> List[str]:
        """ Get all files in a repository with the given suffixes.
        """
        patterns = [f"{repo_path}/**/*.{suffix}" for suffix in suffixes]
        all_files = []
        for pattern in patterns:
            all_files.extend(glob(pattern, recursive=True))
        return all_files

    def get_all_source_files(self, repo_path: os.PathLike,
                             include_headers: bool = False) -> List[str]:
        """ Get all C/C++ source files in a repository.
        """
        exts = ["c", "C", "cc", "cxx", "cpp", "c++", "cu"]
        all_files = self._get_files_by_suffixes(repo_path, exts)
        if include_headers:
            all_files.extend(self.get_all_header_files(repo_path))
        return all_files

    def get_all_header_files(self, repo_path: os.PathLike) -> List[str]:
        """ Get all C/C++ header files in a repository.
        """
        exts = ["h", "H", "hh", "hpp"]
        return self._get_files_by_suffixes(repo_path, exts)

    def get_all_build_files(self, repo_path: os.PathLike) -> List[str]:
        """ Get all build files in a repository.
        """
        exts = ["Makefile", "CMakeLists.txt"]
        return self._get_files_by_suffixes(repo_path, exts)

    def is_cpp_source_file(self, source_file: os.PathLike, include_headers: bool = True) -> bool:
        """ Check if a file is a C/C++ source file.
        """
        ext = os.path.splitext(source_file)[1]
        if ext in [".c", ".C", ".cc", ".cxx", ".cpp", ".c++", ".cu"]:
            return True
        if include_headers and ext in [".h", ".H", ".hh", ".hpp"]:
            return True
        return False

    def get_cpp_source_file_dependencies(self, source_file: os.PathLike) -> Union[List[str], None]:
        """ Uses clang -MM to generate a dependency file on stdout. Parse this file
            and return dependencies.
        """
        ext = os.path.splitext(source_file)[1]

        if ext in [".c", ".C", ".h", ".H"]:
            compiler = "clang"
        elif ext in [".cc", ".cxx", ".cpp", ".c++", ".cu", ".hh", ".hpp"]:
            compiler = "clang++"
        else:
            return None

        cmd = [f"{compiler}", "-MM", "{source_file}"]
        run_result = subprocess.run(cmd, capture_output=True, text=True, check=False)
        if run_result.returncode != 0:
            return None

        deps = run_result.stdout
        print(deps)
        return deps.split(":")[1].strip().split()


    def get_source_file_dependencies_with_llm(self, source_file: os.PathLike,
                                              source_files: Optional[List[os.PathLike]] = None,
                                              num_lines: int = 50) -> Union[List[str], None]:
        """ Uses an LLM to get file dependencies. Give the first num_lines lines
            of the file to the LLM and get back a list of files.
        """
        prompt = "Here are the first {num_lines} lines of the file {source_file}:" + \
            "\n\n{source_lines}\n\n" + \
            "Here are other source files in the same repository:" + \
            "\n\n{source_files}\n\n" + \
            "Which other files in this repository does {source_file} depend on? " + \
            "Please list only the filenames, one per line. For example:" + \
            "\nfoo.cpp\nbar.h\nbaz.cu\n"

        if source_files is None:
            logging.warning("No source files provided to LLM")
            source_files = [""]

        # get the first num_lines lines of the file
        with open(source_file, "r", encoding="UTF-8") as f:
            source_lines = "```\n" + "".join(f.readlines()[:num_lines]) + "\n```"

        prompt = prompt.format(N=num_lines, source_file=source_file,
                               source_lines=source_lines,
                               source_files="\n".join(source_files))
        deps = self._generator.generate(prompt)
        if deps is None:
            return None

        deps = deps.strip().split("\n")
        deps = list(map(str.strip, deps))
        print(f"Dependencies of {source_file}: {deps}")
        if self._interactions_path:
            with open(self._interactions_path, "a", encoding="UTF-8") as f:
                f.write(f"Getting dependencies of {source_file}.\n")
                f.write(f"Prompt:\n{prompt}\n")
                f.write(f"Dependencies:\n{deps}")
        return deps


    def get_source_file_dependencies(self, source_file: os.PathLike,
                                     source_files: Optional[List[os.PathLike]] = None) \
    -> Union[List[str], None]:
        """ Get the dependencies of a source file, first trying to get them statically,
            then using an LLM to choose among the given source files.
        """
        logging.debug("Getting dependencies for %s", source_file)
        # first see if it's a file we can get a dependency for statically
        if self.is_cpp_source_file(source_file, include_headers=True):
            deps = self.get_cpp_source_file_dependencies(source_file)
            if deps is not None:
                return deps

        # if not, try using an LLM to get file dependencies
        deps = self.get_source_file_dependencies_with_llm(source_file, source_files)
        if deps is not None:
            return deps

        # if all else fails, return None
        logging.warning("Could not get dependencies for %s", source_file)
        return None


    def construct_dependency_graph(self, repo_path: os.PathLike) -> List[FileNode]:
        """ Construct a dependency graph for the repository at the given path.
        """
        roots = []

        source_files = self.get_all_source_files(repo_path, include_headers=True)
        build_files = self.get_all_build_files(repo_path)
        logging.debug("Source files: %s", source_files)
        logging.debug("Build files: %s", build_files)

        dependencies = {source_file: self.get_source_file_dependencies(source_file, source_files)
                        for source_file in source_files}
        dependencies.update({build_file: source_files for build_file in build_files})

        for source_file, deps in dependencies.items():
            if deps is None:
                continue

            node = FileNode()
            node.rel_path = source_file.removeprefix(repo_path + "/")
            node.dependencies = [dep for dep in deps if dep in dependencies]
            for dep in node.dependencies:
                dependencies[dep].parents.append(node)
            roots.append(node)

        return roots
