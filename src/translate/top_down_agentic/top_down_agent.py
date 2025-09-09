"""Top-down agent-based approach for translating code repositories.

This module provides an agent-based approach to translating entire code repositories
from one execution model to another (e.g., CUDA to OpenMP or OpenMP to MPI). The
approach is based on the top-down translation method where the agent first determines
the dependency tree of source files and then translates them starting from the root.
Intermediate helper agents are used to split up long files, incorporate context from
further up the tree, and determine filenames and other metadata.

Author: Daniel Nichols
Date: November 2024
"""

import os
import sys
import json
from typing import Dict, Literal, Optional, List, Union, Tuple
import re

# Local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from restate.dependency_agent import DependencyAgent, FileNode, FileType
from restate.chunk_agent import ChunkFileAgent
from restate.context_agent import ContextAgent
from restate import agent_constants as ac
from restate.utils import (
    safe_get_terminal_columns,
    extract_code_block,
    ensure_directory_exists,
    read_file_safely,
    write_file_safely
)
from generator_mixin import GeneratorMixin
from repo import Repo

# Third-party imports
from alive_progress import alive_it


class TopDownAgentTranslator(Translator, GeneratorMixin):
    """Translator for entire repositories using the top-down agent method."""

    def __init__(
            self,
            input_repo: Repo,
            output_repos: List[Union[str, os.PathLike]],
            src_model: str,
            dst_model: str,
            dst_config: dict,
            llm_name: str,
            backend: Literal["openai", "gemini", "hf", "vllm", "local"] = "openai",
            log_interactions: bool = False,
            dry: bool = False,
            hide_progress: bool = False
    ):
        """Initialize the top-down agent translator.

        Args:
            input_repo: The input repository to translate
            output_repos: List of output repository paths
            src_model: Source execution model
            dst_model: Destination execution model
            dst_config: Destination configuration
            llm_name: Name of the LLM to use
            backend: LLM backend to use
            log_interactions: Whether to log LLM interactions
            dry: Whether to run in dry-run mode
            hide_progress: Whether to hide progress bars
        """
        super().__init__(input_repo, output_repos, src_model, dst_model,
                         dst_config, log_interactions, dry, hide_progress)

        self._system_prompt = ac.SYSTEM_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model)

        GeneratorMixin.__init__(self, backend, llm_name, system_prompt=self._system_prompt,
                                async_mode=True)

        self._interactions_paths = self._setup_interaction_logging()
        self._dependency_agent = self._create_dependency_agent()
        self._chunk_file_agent = self._create_chunk_agent()
        self._context_agent = self._create_context_agent()


    def _setup_interaction_logging(self) -> Optional[List[Union[str, os.PathLike]]]:
        """Setup interaction logging paths if enabled."""
        if not self._log_interactions:
            return None

        interactions_paths = []
        for output_repo in self._output_paths:
            interactions_path = os.path.join(output_repo, "interactions.txt")
            interactions_paths.append(interactions_path)
            ensure_directory_exists(interactions_path)

        return interactions_paths


    def _create_dependency_agent(self) -> DependencyAgent:
        """Create and configure the dependency agent."""
        return DependencyAgent(
            generator=self,
            interactions_paths=self._interactions_paths
        )


    def _create_chunk_agent(self) -> ChunkFileAgent:
        """Create and configure the chunk file agent."""
        lang = "c" if self._dst_config["filename_desc"] == "C" else "cpp"
        return ChunkFileAgent(
            generator=self,
            interactions_paths=self._interactions_paths,
            language=lang
        )


    def _create_context_agent(self) -> ContextAgent:
        """Create and configure the context agent."""
        return ContextAgent(
            generator=self,
            output_paths=self._output_paths,
            interactions_paths=self._interactions_paths
        )


    @staticmethod
    def add_args(parser: 'ArgumentParser'): # type: ignore # noqa: F821
        """ Add arguments for the top-down agent translation method.
        """
        parser.add_argument("--restate-backend",
                            choices=["openai", "gemini", "hf", "vllm", "local"],
                            default="openai", help="The backend to use for translation.")
        parser.add_argument("--restate-llm-name",
                            type=str, help="The name of the LLM to use for translation.")


    @staticmethod
    def parse_args(args: 'Namespace') -> Dict[str, str]: # type: ignore # noqa: F821
        """ Parse the arguments for the top-down agent translation method.
        """
        return {
            "backend": args.restate_backend,
            "llm_name": args.restate_llm_name
        }


    def _read_file(self, rel_path: Union[str, os.PathLike]) -> str:
        """Read the contents of a file from the input repository.

        Args:
            rel_path: Relative path to the file

        Returns:
            Contents of the file
        """
        input_file_path = os.path.join(self._input_repo.path, rel_path)
        return read_file_safely(input_file_path)


    def _update_file_ext(self, fname: Union[str, os.PathLike],
                         trigger_rename: Optional[Union[str, os.PathLike]] = None) -> str:
        """Return the filename with updated extension based on the destination model.

        Args:
            fname: Original filename
            trigger_rename: Optional new filename to use

        Returns:
            Updated filename with correct extension
        """
        if trigger_rename:
            fname = trigger_rename

        # Check if file has an extension
        if "." in str(fname):
            name, current_ext = os.path.splitext(fname)

            # Check if the extension is in the dict
            if current_ext in ac.ext_to_type:
                if self._dst_model == "cuda":
                    ext_category = self._dst_model
                else:
                    ext_category = self._input_repo.get_meta_dict()["filename_desc"]
                new_ext = ac.type_to_ext[ext_category.lower()][ac.ext_to_type[current_ext]]
                return name + new_ext
        return str(fname)


    def _write_file(self, rel_path: Union[str, os.PathLike], contents: str, idx: int = 0,
                    trigger_rename: Optional[Union[str, os.PathLike]] = None) -> bool:
        """Write the contents to a file in the output repository.

        Args:
            rel_path: Relative path to the file
            contents: Contents to write
            idx: Index of the output path to use
            trigger_rename: Optional new filename to use

        Returns:
            True if successful, False otherwise
        """
        updated_path = self._update_file_ext(rel_path, trigger_rename=trigger_rename)
        output_file_path = os.path.join(self._output_paths[idx], "repo", updated_path)

        success = write_file_safely(output_file_path, contents)
        if success:
            print(f"Wrote file {output_file_path}")
        return success


    def _write_metadata(self, repo_path: Union[str, os.PathLike]) -> bool:
        """Write out experiment_metadata.json adjacent to repo path.

        Args:
            repo_path: Path to the repository

        Returns:
            True if successful, False otherwise
        """
        exp_meta_fpath = os.path.join(repo_path, "..", "experiment_metadata.json")

        try:
            exp_meta_dict = {
                "app": self._input_repo.get_meta_dict()["app"],
                "prompt_strategy": "restate",
                "llm_name": self._llm_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": int(str(repo_path).split("/")[-2][7:]),
                "path": str(repo_path),
                "inference_stats": self.get_stats()
            }

            success = write_file_safely(exp_meta_fpath, json.dumps(exp_meta_dict, indent=4))
            if success:
                print(f"Wrote translation experiment metadata to {exp_meta_fpath}")
            return success

        except Exception as e:
            print(f"Error writing metadata: {e}")
            return False


    def _log_interaction(self, prompts: List[str], responses: List[str],
                         reasonings: List[Union[str, None]]) -> None:
        """Log the prompt and raw LLM output to text files.

        Args:
            prompts: List of prompts sent to the LLM
            responses: List of responses from the LLM
            reasonings: List of reasoning outputs (may be None)
        """
        if not self._interactions_paths:
            return

        for i, output_path in enumerate(self._interactions_paths):
            try:
                with open(output_path, 'a', encoding="UTF-8") as f:
                    f.write("PROMPT:\n")
                    f.write(prompts[i] + "\n")
                    if reasonings[i] is not None:
                        f.write("REASONING:\n")
                        f.write(reasonings[i] + "\n")
                    f.write("RESPONSE:\n")
                    f.write(responses[i] + "\n\n")
            except Exception as e:
                print(f"Error logging interaction to {output_path}: {e}")


    def translate(self):
        """Use the top-down method to translate the entire repository."""
        print(f"Constructing dependency graph on {self._input_repo.path}...")

        # Build dependency graph and prepare for translation
        dep_graph = self._dependency_agent.construct_dependency_graph(self._input_repo.path)
        file_tree = self._input_repo.get_file_tree_str()
        target_dep_graph = DependencyAgent.make_target_graph(dep_graph, self._update_file_ext)

        # Translate each file in dependency order
        self._translate_all_files(dep_graph, target_dep_graph, file_tree)

        # Write experiment metadata
        self._write_all_metadata()


    def _translate_all_files(self, dep_graph: List[FileNode],
                           target_dep_graph: List[FileNode],
                           file_tree: str) -> None:
        """Translate all files in the dependency graph."""
        max_cols = safe_get_terminal_columns()

        for node in alive_it(dep_graph,
                             title="Translating files",
                             max_cols=max_cols,
                             disable=self._hide_progress):
            if node.filetype == FileType.BUILD:
                self._translate_node(node, graph=target_dep_graph, tree=file_tree)
            else:
                self._translate_node(node)


    def _write_all_metadata(self) -> None:
        """Write experiment metadata for all output repositories."""
        for output_repo in self._output_paths:
            self._write_metadata(os.path.join(output_repo, "repo"))


    def _translate_node(self, node: FileNode, graph: Optional[List[FileNode]] = None,
                        tree: Optional[str] = None) -> None:
        """Translate a single file node using context from its dependencies.

        Args:
            node: The file node to translate
            graph: Optional dependency graph for build files
            tree: Optional file tree for build files
        """
        print(f"Translating file {node.rel_path}...")

        # Get source code and context
        source_code = self._read_file(node.rel_path)
        contexts = self._context_agent.get_contexts(node.dependencies, node, self._dst_model)

        # Translate the file (chunked if necessary)
        translations, trigger_rename = self._translate_file_content(
            source_code, contexts, node, graph, tree
        )

        # Write translations to output files
        self._write_translations(node, translations, trigger_rename)


    def _translate_file_content(self, source_code: str, contexts: List[str],
                               node: FileNode, graph: Optional[List[FileNode]] = None,
                               tree: Optional[str] = None) -> Tuple[List[str], Optional[str]]:
        """Translate file content, handling chunking if necessary."""
        chunks = self._chunk_file_agent.chunk_file(source_code)

        if len(chunks) == 1:
            return self._translate_single_chunk(chunks[0], contexts, node, graph, tree)
        else:
            return self._translate_multiple_chunks(chunks, contexts, node, graph, tree)


    def _translate_single_chunk(self, chunk: str, contexts: List[str],
                               node: FileNode, graph: Optional[List[FileNode]] = None,
                               tree: Optional[str] = None) -> Tuple[List[str], Optional[str]]:
        """Translate a single chunk of code."""
        print("Requesting whole file translation...")
        responses, trigger_rename = self._get_translations(contexts, chunk, node, graph, tree)
        translations = [extract_code_block(response) for response in responses]
        return translations, trigger_rename


    def _translate_multiple_chunks(self, chunks: List[str], contexts: List[str],
                                  node: FileNode, graph: Optional[List[FileNode]] = None,
                                  tree: Optional[str] = None) -> Tuple[List[str], Optional[str]]:
        """Translate multiple chunks of code."""
        translations = []
        prev_chunks = []
        trigger_rename = None

        for i, chunk in enumerate(chunks):
            print(f"Requesting chunk translation... [{i + 1}/{len(chunks)}]")
            responses, trigger_rename = self._get_translations(
                contexts, chunk, node, graph, tree, prev_chunks=prev_chunks
            )
            chunk_translations = [extract_code_block(response) for response in responses]

            if translations:
                translations = self._merge_chunk_translations(translations, chunk_translations)
            else:
                translations = chunk_translations

            prev_chunks = chunk_translations

        return translations, trigger_rename


    def _merge_chunk_translations(self, existing: List[str], new: List[str]) -> List[str]:
        """Merge new chunk translations with existing ones."""
        return [
            (existing[i] if existing[i] is not None else "") + "\n" +
            (new[i] if new[i] is not None else "")
            for i in range(len(existing))
        ]


    def _write_translations(self, node: FileNode, translations: List[str],
                           trigger_rename: Optional[str]) -> None:
        """Write translations to output files."""
        for i, translation in enumerate(translations):
            if translation is not None:
                self._write_file(node.rel_path, translation, i, trigger_rename)


    def _get_translations(self, contexts: List[str], source_code: str, file: FileNode,
                          graph: Optional[List[FileNode]] = None,
                          tree: Optional[str] = None,
                          prev_chunks: List[Union[str, None]] = []) -> Tuple[List[str], Optional[str]]:
        """Get the translation for a region of code using the provided context.

        Args:
            contexts: List of context strings for each output path
            source_code: The source code to translate
            file: The file node being translated
            graph: Optional dependency graph for build files
            tree: Optional file tree for build files
            prev_chunks: Previous chunk translations for multi-chunk files

        Returns:
            Tuple of (responses, trigger_rename)
        """
        # Build base prompts
        prompts = self._build_base_prompts(contexts, source_code, file)

        # Add chunk addendum if needed
        prompts = self._add_chunk_addendum(prompts, prev_chunks)

        # Add main file addendum if needed
        prompts = self._add_main_addendum(prompts, file)

        # Add build file addendum if needed
        trigger_rename = self._add_build_addendum(prompts, file, graph, tree)

        # Generate translations
        responses, reasonings = self._generate_translations(prompts)

        # Log interactions if enabled
        if self._log_interactions:
            self._log_interaction(prompts, responses, reasonings)

        return responses, trigger_rename


    def _build_base_prompts(self, contexts: List[str], source_code: str, file: FileNode) -> List[str]:
        """Build the base prompts for translation."""
        prompt_config_dst = self._dst_config
        filename = file.rel_path
        filename_desc = prompt_config_dst["filename_desc"]
        exts_str = ", ".join(v for v in ac.type_to_ext[filename_desc.lower()].values())

        return [
            ac.PROMPT_TEMPLATE.format(
                src_model=self._src_model,
                filename=filename,
                dst_model=self._dst_model,
                source_code=source_code,
                context=c,
                exts=exts_str,
                filename_desc=filename_desc
            ) for c in contexts
        ]


    def _add_chunk_addendum(self, prompts: List[str], prev_chunks: List[Union[str, None]]) -> List[str]:
        """Add chunk addendum to prompts if previous chunks exist."""
        if not prev_chunks:
            return prompts

        return [
            p + "\n" + ac.CHUNK_ADDENDUM.format(
                prev_chunk=prev_chunk,
                src_model=self._src_model,
                dst_model=self._dst_model
            ) for p, prev_chunk in zip(prompts, prev_chunks)
        ]


    def _add_main_addendum(self, prompts: List[str], file: FileNode) -> List[str]:
        """Add main file addendum to prompts if this is the main file."""
        prompt_config_src = self._input_repo.get_meta_dict()
        prompt_config_dst = self._dst_config

        if file.rel_path != prompt_config_src["main_filename"]:
            return prompts

        return [
            p + "\n" + ac.MAIN_ADDENDUM.format(
                ex_run_cmd=prompt_config_dst["ex_run_cmd"],
                ex_run_desc=prompt_config_dst["ex_run_desc"]
            ) for p in prompts
        ]


    def _add_build_addendum(self, prompts: List[str], file: FileNode,
                           graph: Optional[List[FileNode]], tree: Optional[str]) -> Optional[str]:
        """Add build file addendum to prompts if this is a build file."""
        prompt_config_src = self._input_repo.get_meta_dict()
        prompt_config_dst = self._dst_config
        filename = file.rel_path

        # Determine the key filename for build addendum
        key_filename = prompt_config_src["build_filename"]
        trigger_rename = None

        if prompt_config_dst["build_filename"] != prompt_config_src["build_filename"]:
            trigger_rename = prompt_config_dst["build_filename"]

            # Check if we should use an extra build file
            if ("extra_build_files" in prompt_config_src and
                prompt_config_dst["build_filename"] in prompt_config_src["extra_build_files"]):
                key_filename = prompt_config_dst["build_filename"]

        if filename == key_filename:
            filename_desc = prompt_config_dst["filename_desc"]
            exts_str = ", ".join(v for v in ac.type_to_ext[filename_desc.lower()].values())

            build_addendum = ac.BUILD_ADDENDUM.format(
                build_filename=key_filename,
                new_build_filename=prompt_config_dst["build_filename"],
                dst_model=self._dst_model,
                exts=exts_str,
                filename_desc=filename_desc,
                ex_build_cmd=prompt_config_dst["ex_build_cmd"],
                ex_build_desc=prompt_config_dst["ex_build_desc"],
                dep_graph=DependencyAgent.graph_to_str(graph),
                file_tree=tree
            )

            for i in range(len(prompts)):
                prompts[i] += "\n" + build_addendum

            return trigger_rename
        else:
            return None


    def _generate_translations(self, prompts: List[str]) -> Tuple[List[str], List[Union[str, None]]]:
        """Generate translations using the LLM."""
        response_obs = self.generate_async(prompts, temperature=0.2, top_p=0.95)
        responses = [r.response for r in response_obs]
        reasonings = [r.reasoning for r in response_obs]
        return responses, reasonings
