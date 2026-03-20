"""Context Agent for extracting relevant context from translated files.

This agent is responsible for gathering context from already-translated files
to help with the translation of subsequent files in the dependency chain.
"""

import logging
import os
from typing import List, Optional, Union
from generator_mixin import GeneratorMixin

logger = logging.getLogger("pareval-repo")
from .dependency_agent import FileNode
from .utils import read_file_safely


class ContextAgent:
    """Agent responsible for extracting context from translated files."""

    def __init__(self,
                 generator: GeneratorMixin,
                 output_paths: List[Union[str, os.PathLike]],
                 interactions_paths: Optional[List[Union[str, os.PathLike]]] = None):
        """Initialize the context agent.

        Args:
            generator: The generator mixin for LLM interactions
            output_paths: List of output paths where translated files are stored
            interactions_paths: Optional paths for logging interactions
        """
        self._generator = generator
        self._interactions_paths = interactions_paths
        self._output_paths = output_paths


    def get_contexts(self, dependencies: List[FileNode],
                     node: FileNode, dst_model: str) -> List[str]:
        """Extract context for the translation task from dependent files.

        Args:
            dependencies: List of files that the current file depends on
            node: The current file node being translated
            dst_model: The destination execution model

        Returns:
            List of context strings for each output path
        """
        logger.debug("Extracting context from %d dependent file(s)...", len(dependencies))

        if not dependencies:
            return ["" for _ in self._output_paths]

        prompts = self._build_context_prompts(dependencies, node, dst_model)
        return self._generate_contexts(prompts)


    def _build_context_prompts(self, dependencies: List[FileNode],
                              node: FileNode, dst_model: str) -> List[str]:
        """Build prompts for context extraction for each output path."""
        prompts = []

        for output_path in self._output_paths:
            translated_codes = self._collect_translated_codes(dependencies, output_path)
            prompt = self._create_context_prompt(translated_codes, node, dst_model)
            prompts.append(prompt)

        return prompts


    def _collect_translated_codes(self, dependencies: List[FileNode],
                                 output_path: Union[str, os.PathLike]) -> List[str]:
        """Collect translated code from dependent files."""
        translated_codes = []

        for dependency in dependencies:
            translated_code = self._read_translated_file(dependency.rel_path, output_path)
            if translated_code:
                translated_codes.append(f"File: {dependency.rel_path}\n{translated_code}")

        return translated_codes


    def _create_context_prompt(self, translated_codes: List[str],
                              node: FileNode, dst_model: str) -> str:
        """Create the prompt for context extraction."""
        combined_translations = "\n\n".join(translated_codes)

        return (
            f"You are assisting with the translation of an application to {dst_model}. "
            f"The next file in the application to translate is {node.rel_path}. "
            f"Below are the files already translated:\n\n"
            f"{combined_translations}\n\n"
            f"Please extract any code snippets from the above translated files that may be "
            f"relevant to translating {node.rel_path}. Do not provide any new code or "
            f"translations, just rewrite the relevant context in the form of code snippets "
            f"with some explanation."
        )


    def _generate_contexts(self, prompts: List[str]) -> List[str]:
        """Generate context responses using the LLM."""
        try:
            response_obs = self._generator.generate_async(prompts)
            return [r.response.strip() if r.response else "" for r in response_obs]
        except Exception as e:
            logger.error("Error generating contexts: %s", e)
            return ["" for _ in prompts]


    def _read_translated_file(self, rel_path: Union[str, os.PathLike],
                             output_path: Union[str, os.PathLike]) -> str:
        """Read the translated file from the output path.

        Args:
            rel_path: Relative path to the file
            output_path: Base output path where translated files are stored

        Returns:
            Contents of the translated file, or empty string if not found
        """
        output_file_path = os.path.join(output_path, rel_path)
        return read_file_safely(output_file_path, encoding='utf-8')
