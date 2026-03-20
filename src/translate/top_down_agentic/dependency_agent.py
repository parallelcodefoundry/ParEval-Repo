"""Dependency Agent for constructing file dependency graphs.

This agent handles constructing a file dependency graph for a repository.
For C/C++ files, it uses clang -MMD to generate dependency information for each
source file. This is then used to construct a dependency graph for the entire
repository. Build files depend on the entire repository, while metadata files
such as READMEs and LICENSEs are disconnected components in the graph.

Author: Daniel Nichols
Date: November 2024
"""

import logging
import os
import subprocess

logger = logging.getLogger("pareval-repo")
from glob import glob
from typing import List, Optional, Union, Callable
from graphlib import TopologicalSorter, CycleError

# Local imports
from generator_mixin import GeneratorMixin
from .utils import FileType, is_cpp_source_file, format_dependency_list


class FileNode:
    """A node in a file dependency graph."""

    def __init__(self, rel_path: Union[str, os.PathLike] = "", filetype: Optional[FileType] = None):
        """Initialize a file node.

        Args:
            rel_path: Relative path to the file
            filetype: Optional file type, will be determined automatically if not provided
        """
        self.rel_path = str(rel_path)
        self.dependencies: List['FileNode'] = []  # Files this file depends on
        self.dependents: List['FileNode'] = []    # Files that depend on this file
        self.filetype = filetype or self._get_filetype()

    def _get_filetype(self) -> FileType:
        """Get the file type based on the file extension and name."""
        from .utils import get_file_type
        return get_file_type(self.rel_path)

    @property
    def basename(self) -> str:
        """Get the basename of the file."""
        return os.path.basename(self.rel_path)

    def format(self) -> str:
        """Format the file node as a readable string."""
        dep_paths = [dep.rel_path for dep in self.dependencies]
        return f"{self.rel_path} depends on {format_dependency_list(dep_paths)}"

    def __repr__(self) -> str:
        """String representation of the file node."""
        return f"FileNode('{self.rel_path}', {self.filetype.name})"


class DependencyAgent:
    """Agent for constructing file dependency graphs for repositories."""

    def __init__(self,
                 generator: GeneratorMixin,
                 interactions_paths: Optional[List[os.PathLike]] = None):
        """Initialize the dependency agent.

        Args:
            generator: The generator mixin for LLM interactions
            interactions_paths: Optional paths for logging interactions
        """
        self._generator = generator
        self._interactions_paths = interactions_paths


    def _get_files_by_suffixes(self, repo_path: Union[str, os.PathLike],
                               suffixes: List[str]) -> List[str]:
        """Get all files in a repository with the given suffixes.

        Args:
            repo_path: Path to the repository
            suffixes: List of file suffixes to search for

        Returns:
            List of matching file paths
        """
        patterns = [f"{repo_path}/**/*.{suffix}" for suffix in suffixes]
        all_files = []
        for pattern in patterns:
            all_files.extend(glob(pattern, recursive=True))
        return all_files


    def get_all_source_files(self, repo_path: Union[str, os.PathLike],
                             include_headers: bool = True) -> List[str]:
        """Get all C/C++ source files in a repository.

        Args:
            repo_path: Path to the repository
            include_headers: Whether to include header files

        Returns:
            List of source file paths
        """
        exts = ["c", "C", "cc", "cxx", "cpp", "c++", "cu"]
        all_files = self._get_files_by_suffixes(repo_path, exts)
        if include_headers:
            all_files.extend(self.get_all_header_files(repo_path))
        return all_files


    def get_all_header_files(self, repo_path: Union[str, os.PathLike]) -> List[str]:
        """Get all C/C++ header files in a repository.

        Args:
            repo_path: Path to the repository

        Returns:
            List of header file paths
        """
        exts = ["h", "H", "hh", "hpp", "cuh"]
        return self._get_files_by_suffixes(repo_path, exts)


    def get_all_build_files(self, repo_path: Union[str, os.PathLike]) -> List[str]:
        """Get all build files in a repository.

        Args:
            repo_path: Path to the repository

        Returns:
            List of build file paths
        """
        exts = ["make"]  # Remove the dot for glob pattern
        allfiles = self._get_files_by_suffixes(repo_path, exts)
        names = ["Makefile", "CMakeLists.txt"]
        for name in names:
            allfiles.extend(glob(f"{repo_path}/**/{name}", recursive=True))
        return allfiles


    def _trim_dependencies(self, dependencies: dict, all_files: List[str],
                           repo_path: Union[str, os.PathLike]) -> dict:
        """Remove dependencies that are not in the list of all files.

        Args:
            dependencies: Dictionary mapping files to their dependencies
            all_files: List of all valid files in the repository
            repo_path: Path to the repository

        Returns:
            Dictionary with trimmed dependencies and relative paths
        """
        trimmed = {}
        repo_prefix = f"{repo_path}/"

        for source_file, deps in dependencies.items():
            if not deps:
                trimmed[source_file.removeprefix(repo_prefix)] = []
                continue

            trimmed_deps = [
                dep.removeprefix(repo_prefix)
                for dep in deps
                if dep in all_files and dep != source_file
            ]
            trimmed[source_file.removeprefix(repo_prefix)] = trimmed_deps
        return trimmed


    def is_cpp_source_file(self, source_file: Union[str, os.PathLike],
                           include_headers: bool = True) -> bool:
        """Check if a file is a C/C++ source file.

        Args:
            source_file: Path to the file to check
            include_headers: Whether to include header files

        Returns:
            True if the file is a C/C++ source file
        """
        return is_cpp_source_file(source_file, include_headers)


    def get_cpp_source_file_dependencies(self, source_file: Union[str, os.PathLike],
                                         repo_path: Union[str, os.PathLike]) -> Optional[List[str]]:
        """Use clang -MM to generate dependency information for a source file.

        Args:
            source_file: Path to the source file
            repo_path: Path to the repository root

        Returns:
            List of dependencies or None if failed
        """
        logger.debug("Getting dependencies for %s using clang -MM", source_file)

        compiler = self._get_compiler_for_file(source_file)
        if not compiler:
            return None

        cmd = self._build_clang_command(compiler, source_file, repo_path)
        return self._run_clang_command(cmd, repo_path)


    def _get_compiler_for_file(self, source_file: Union[str, os.PathLike]) -> Optional[str]:
        """Get the appropriate compiler for the given file type."""
        ext = os.path.splitext(source_file)[1]

        if ext in [".c", ".C", ".h", ".H"]:
            return "clang"
        elif ext in [".cc", ".cxx", ".cpp", ".c++", ".cu", ".hh", ".hpp", ".cuh"]:
            return "clang++"
        else:
            return None


    def _build_clang_command(self, compiler: str, source_file: Union[str, os.PathLike],
                           repo_path: Union[str, os.PathLike]) -> List[str]:
        """Build the clang command with include directories."""
        cmd = [compiler, "-MM", str(source_file)]

        # Add -Ipath for every subdirectory in the repo
        # TODO: Find a better way to get include directories from build system
        try:
            include_dirs = [
                d for d in os.listdir(repo_path)
                if os.path.isdir(os.path.join(repo_path, d))
            ]
            for d in include_dirs:
                cmd.append(f"-I{d}")
        except OSError as e:
            logger.warning("Could not list directories in %s: %s", repo_path, e)

        return cmd


    def _run_clang_command(self, cmd: List[str], repo_path: Union[str, os.PathLike]) -> Optional[List[str]]:
        """Run the clang command and parse the output."""
        try:
            run_result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=False,
                cwd=repo_path
            )

            if run_result.returncode != 0:
                logger.warning("clang -MM failed: %s", run_result.stderr.strip())
                return None

            deps = run_result.stdout.strip()
            if not deps:
                return []

            # Parse the dependency output
            if ":" in deps:
                deps_part = deps.split(":", 1)[1].strip()
                return deps_part.replace("\\", "").split()
            return []

        except Exception as e:
            logger.error("Error running clang command: %s", e)
            return None


    def get_source_file_dependencies_with_llm(self, source_file: Union[str, os.PathLike],
                                              source_files: Optional[List[Union[str, os.PathLike]]] = None,
                                              num_lines: int = 50) -> Optional[List[str]]:
        """Use an LLM to determine file dependencies.

        Args:
            source_file: Path to the source file
            source_files: Optional list of other source files in the repository
            num_lines: Number of lines to read from the file for analysis

        Returns:
            List of dependencies or None if failed
        """
        logger.debug("Getting dependencies for %s using LLM", source_file)

        if source_files is None:
            logger.warning("No source files provided to LLM dependency analysis.")
            source_files = [""]

        try:
            source_lines = self._read_file_lines(source_file, num_lines)
            prompt = self._build_llm_dependency_prompt(source_file, source_lines, source_files)

            response = self._generator.generate(prompt)[0].response
            if not response:
                return None

            deps = self._parse_llm_dependencies(response)
            self._log_llm_interaction(source_file, prompt, deps)

            return deps

        except Exception as e:
            logger.error("Error getting LLM dependencies for %s: %s", source_file, e)
            return None


    def _read_file_lines(self, source_file: Union[str, os.PathLike], num_lines: int) -> str:
        """Read the first num_lines of a file."""
        try:
            with open(source_file, "r", encoding="UTF-8") as f:
                lines = f.readlines()[:num_lines]
                return "```\n" + "".join(lines) + "\n```"
        except Exception as e:
            logger.error("Error reading file %s: %s", source_file, e)
            return "```\n```"


    def _build_llm_dependency_prompt(self, source_file: Union[str, os.PathLike],
                                   source_lines: str,
                                   source_files: List[Union[str, os.PathLike]]) -> str:
        """Build the prompt for LLM dependency analysis."""
        return (
            f"Here are the first lines of the file {source_file}:\n\n"
            f"{source_lines}\n\n"
            f"Here are other source files in the same repository:\n\n"
            f"{chr(10).join(str(f) for f in source_files)}\n\n"
            f"Which other files in this repository does {source_file} depend on? "
            f"Please list only the filenames, one per line. For example:\n"
            f"foo.cpp\nbar.h\nbaz.cu\n"
        )


    def _parse_llm_dependencies(self, response: str) -> List[str]:
        """Parse the LLM response to extract dependencies."""
        deps = response.strip().split("\n")
        deps = [dep.strip() for dep in deps if dep.strip()]
        logger.debug("LLM identified dependencies: %s", format_dependency_list(deps))
        return deps


    def _log_llm_interaction(self, source_file: Union[str, os.PathLike],
                           prompt: str, deps: List[str]) -> None:
        """Log the LLM interaction for debugging."""
        if not self._interactions_paths:
            return

        for path in self._interactions_paths:
            try:
                with open(path, "a", encoding="UTF-8") as f:
                    f.write(f"Getting dependencies of {source_file}.\n")
                    f.write(f"Prompt:\n{prompt}\n")
                    f.write(f"Dependencies:\n{format_dependency_list(deps)}\n\n")
            except Exception as e:
                logger.warning("Could not log interaction to %s: %s", path, e)


    def get_source_file_dependencies(self, source_file: Union[str, os.PathLike],
                                     source_files: List[Union[str, os.PathLike]],
                                     repo_path: Union[str, os.PathLike]) -> Optional[List[str]]:
        """Get the dependencies of a source file using static analysis or LLM.

        Args:
            source_file: Path to the source file
            source_files: List of all source files in the repository
            repo_path: Path to the repository root

        Returns:
            List of dependencies or None if failed
        """
        # First try static analysis for C/C++ files
        if self.is_cpp_source_file(source_file, include_headers=True):
            deps = self.get_cpp_source_file_dependencies(source_file, repo_path)
            if deps is not None:
                return [os.path.join(repo_path, dep) for dep in deps]

        # Fallback to LLM-based analysis
        deps = self.get_source_file_dependencies_with_llm(source_file, source_files)
        if deps is not None:
            return [os.path.join(repo_path, dep) for dep in deps]

        logger.warning("Could not get dependencies for %s", source_file)
        return None


    def construct_dependency_graph(self, repo_path: Union[str, os.PathLike]) -> List[FileNode]:
        """Construct a dependency graph for the repository at the given path.

        Args:
            repo_path: Path to the repository

        Returns:
            List of root FileNodes in the dependency graph
        """
        # Get all files in the repository
        source_files, build_files, all_files = self._collect_repository_files(repo_path)

        # Build dependency dictionary
        dependencies = self._build_dependency_dictionary(source_files, build_files, repo_path)
        dependencies = self._trim_dependencies(dependencies, all_files, repo_path)

        # Sort files topologically
        sorted_files = self._topologically_sort_files(dependencies)

        # Construct the graph
        roots = self._construct_file_nodes(sorted_files, dependencies)

        logger.debug("Constructed dependency graph with %d root(s): %s",
                     len(roots), [root.rel_path for root in roots])
        return roots


    def _collect_repository_files(self, repo_path: Union[str, os.PathLike]) -> tuple:
        """Collect all source and build files in the repository."""
        source_files = self.get_all_source_files(repo_path, include_headers=True)
        build_files = self.get_all_build_files(repo_path)
        all_files = list(set(source_files + build_files))

        logger.debug("Source files: %s", source_files)
        logger.debug("Build files: %s", build_files)

        return source_files, build_files, all_files


    def _build_dependency_dictionary(self, source_files: List[str],
                                   build_files: List[str],
                                   repo_path: Union[str, os.PathLike]) -> dict:
        """Build the dependency dictionary for all files."""
        dependencies = {}

        # Get dependencies for each source file
        for source_file in source_files:
            deps = self.get_source_file_dependencies(source_file, source_files, repo_path)
            dependencies[source_file] = deps or []

        # Build files depend on all source files
        for build_file in build_files:
            dependencies[build_file] = source_files

        return dependencies


    def _topologically_sort_files(self, dependencies: dict) -> List[str]:
        """Sort files topologically based on their dependencies."""
        ts = TopologicalSorter()

        for source_file, deps in dependencies.items():
            ts.add(source_file, *deps)

        try:
            return list(ts.static_order())
        except CycleError:
            # Handle cycles by sorting by dependency count
            logging.warning("Cycle detected in dependency graph, using fallback sorting")
            return list(dict(sorted(dependencies.items(), key=lambda x: len(x[1]))).keys())


    def _construct_file_nodes(self, sorted_files: List[str],
                            dependencies: dict) -> List[FileNode]:
        """Construct FileNode objects from the sorted files and dependencies."""
        roots = []

        for source_file in sorted_files:
            node = FileNode(source_file)
            node.dependencies = [FileNode(dep) for dep in dependencies[source_file]]

            # Set up bidirectional relationships
            for dep in node.dependencies:
                dep.dependents.append(node)

            roots.append(node)

        return roots


    @staticmethod
    def make_target_graph(old_graph: List[FileNode],
                          path_update_fn: Callable) -> List[FileNode]:
        """Create a new graph from the old graph using the path update function.

        Args:
            old_graph: The original dependency graph
            path_update_fn: Function to update file paths

        Returns:
            New graph with updated paths
        """
        new_graph = []

        for node in old_graph:
            new_node = FileNode(path_update_fn(node.rel_path))
            new_node.dependencies = [
                FileNode(path_update_fn(dep.rel_path))
                for dep in node.dependencies
            ]

            # Set up bidirectional relationships
            for dep in new_node.dependencies:
                dep.dependents.append(new_node)

            new_graph.append(new_node)

        return new_graph


    @staticmethod
    def graph_to_str(graph: List[FileNode]) -> str:
        """Format the graph as a readable string.

        Args:
            graph: List of FileNodes to format

        Returns:
            Formatted string representation of the graph
        """
        return "\n".join(node.format() for node in graph)
