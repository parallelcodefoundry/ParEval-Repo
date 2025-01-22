""" Dependency Agent. Handles constructing a file dependency graph for a respository.

    For C/C++ uses clang -MMD to generate a dependency file for each source file. This
    is then used to construct a dependecy graph for the entire repository. Build files
    depend on the entire repository. Other metadata files such as READMEs and LICENSEs
    are disconnected components in the graph.

    author: Daniel Nichols
    date: November 2024
"""
# std imports
from ctypes import Union
from glob import glob
import logging
import os
import subprocess
from typing import List, Union, Optional

# local imports
from generator_mixin import GeneratorMixin


class FileNode:
    rel_path: str
    dependencies: List['FileNode'] = []
    parents: List['FileNode'] = []

    @property
    def basename(self) -> str:
        return os.path.basename(self.rel_path)



class DependencyAgent:

    def __init__(self, generator: GeneratorMixin, interactions_path: os.PathLike = None):
        self._generator = generator
        self._interactions_path = interactions_path

    def get_all_source_files(self, repo_path: os.PathLike) -> List[str]:
        exts = ["c", "C", "cc", "cxx", "cpp", "c++", "cu"]
        patterns = [f"{repo_path}/**/*.{ext}" for ext in exts]
        all_files = []
        for pattern in patterns:
            all_files.extend(glob(pattern, recursive=True))
        return all_files

    def is_cpp_source_file(self, source_file: os.PathLike, include_headers: bool = True) -> bool:
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

        if ext in [".c", ".C"]:
            compiler = "clang"
        elif ext in [".cc", ".cxx", ".cpp", ".c++", ".cu"]:
            compiler = "clang++"
        else:
            return None

        cmd = [f"{compiler}", "-MM", "{source_file}"]
        run_result = subprocess.run(cmd, capture_output=True, text=True)
        if run_result.returncode != 0:
            return None

        deps = run_result.stdout
        print(deps)
        return deps.split(":")[1].strip().split()


    def get_source_file_dependencies_with_llm(self, source_file: os.PathLike, source_files: Optional[List[os.PathLike]] = None, N: int = 50) -> Union[List[str], None]:
        """ Uses an LLM to get file dependencies.
            Give the first N lines of the file to the LLM and get back a list of files
        """
        prompt = "Here are the first {N} lines of the file {source_file}:\n\n{source_lines}\n\nHere are other source files in the same repository:\n\n{source_files}\n\n" + \
            "Which other files in this repository does {source_file} depend on? Please list only the filenames, one per line. For example: \nfoo.cpp\nbar.h\nbaz.cu\n"

        if source_files is None:
            logging.warning("No source files provided to LLM")
            source_files = [""]

        # get the first N lines of the file
        with open(source_file, "r") as f:
            source_lines = "```\n" + "".join(f.readlines()[:N]) + "\n```"

        prompt = prompt.format(N=N, source_file=source_file, source_lines=source_lines, source_files="\n".join(source_files))
        deps = self._generator.generate(prompt)
        if deps is None:
            return None

        deps = deps.strip().split("\n")
        deps = list(map(str.strip, deps))
        print(f"Dependencies of {source_file}: {deps}")
        if self._interactions_path:
            with open(self._interactions_path, "a") as f:
                f.write(f"Getting dependencies of {source_file}.\n")
                f.write(f"Prompt:\n{prompt}\n")
                f.write(f"Dependencies:\n{deps}")
        return deps


    def get_source_file_dependencies(self, source_file: os.PathLike, source_files: Optional[List[os.PathLike]] = None) -> Union[List[str], None]:
        """ Get the dependencies of a source file, first trying to get them statically,
            then using an LLM to choose among the given source files.
        """
        # first see if it's a file we can get a dependency for statically
        if self.is_cpp_source_file(source_file):
            deps = self.get_cpp_source_file_dependencies(source_file)
            if deps is not None:
                return deps

        # if not, try using an LLM to get file dependencies
        deps = self.get_source_file_dependencies_with_llm(source_file, source_files)
        if deps is not None:
            return deps

        # if all else fails, return None
        logging.warning(f"Could not get dependencies for {source_file}")
        return None



    def construct_dependency_graph(self, repo_path: os.PathLike) -> List[FileNode]:
        """ Construct a dependency graph for the repository at the given path.
        """
        roots = []

        source_files = self.get_all_source_files(repo_path)
        #header_files = self.get_all_header_files(repo_path)
        #build_files = self.get_all_build_files(repo_path)
        print(source_files)
        dependencies = {source_file: self.get_source_file_dependencies(source_file, source_files) for source_file in source_files}

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
