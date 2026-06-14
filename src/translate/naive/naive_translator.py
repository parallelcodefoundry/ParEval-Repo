""" Naively translate file-by-file from one repository to the other. Include
    as much context in the prompt as possible.
"""
# std imports
import logging
import os
import sys
import re
import json
from typing import Dict, Tuple, Union, Literal, List, Optional
from pathlib import Path

# tpl imports
from alive_progress import alive_it

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from generator_mixin import GeneratorMixin, GenericResponse
from repo import Repo
from naive import naive_constants as nc
from top_down_agentic.chunk_agent import ChunkFileAgent

logger = logging.getLogger("pareval-repo")

# Constants
DEFAULT_TERMINAL_COLS = 80
DEFAULT_CHUNK_MAX_TOKENS = 1024
CODE_BLOCK_PATTERN = re.compile(r"```(?:[+\w]+)?\n(.*?)\n```", re.DOTALL)
INTERACTIONS_DIR = "interactions"
EXPERIMENT_METADATA_FILE = "experiment_metadata.json"


class NaiveTranslator(Translator, GeneratorMixin):
    """ Class to naively translate file-by-file from one repository to the other.

    This translator uses a naive approach where each file is translated individually
    with full context from the entire repository included in the prompt.
    """

    _dst_config: dict
    _chunk_agent: Optional[ChunkFileAgent] = None

    def __init__(
            self,
            input_repo: Repo,
            output_repos: List[os.PathLike],
            src_model: str,
            dst_model: str,
            dst_config: dict,
            llm_name: str,
            backend: Literal["openai", "gemini", "hf", "vllm", "local"] = "openai",
            enable_chunking: bool = False,
            log_interactions: bool = False,
            dry: bool = False,
            hide_progress: bool = False,
            api_key: Optional[str] = None,
            api_base_url: Optional[str] = None,
            vllm_environment: Optional[str] = None,
            vllm_yaml_config: Optional[str] = None,
            vllm_keepalive_id: Optional[str] = None,
    ):
        # Validate inputs
        self._validate_inputs(input_repo, output_repos, src_model, dst_model,
                             dst_config, llm_name, backend)

        super().__init__(input_repo, output_repos, src_model, dst_model,
                         dst_config, log_interactions, dry, hide_progress)

        GeneratorMixin.__init__(self, backend, llm_name,
                                system_prompt=self._get_system_prompt(),
                                api_key=api_key,
                                api_base_url=api_base_url,
                                vllm_environment=vllm_environment,
                                vllm_yaml_config=vllm_yaml_config,
                                vllm_keepalive_id=vllm_keepalive_id)

        if enable_chunking:
            self._chunk_agent = ChunkFileAgent(self, max_tokens=DEFAULT_CHUNK_MAX_TOKENS)


    def _validate_inputs(self, input_repo: Repo, output_repos: List[os.PathLike],
                        src_model: str, dst_model: str, dst_config: dict,
                        llm_name: str, backend: str) -> None:
        """Validate input parameters.

        Args:
            input_repo: Input repository
            output_repos: List of output repository paths
            src_model: Source model name
            dst_model: Destination model name
            dst_config: Destination configuration
            llm_name: LLM name
            backend: Backend type

        Raises:
            ValueError: If any validation fails
        """
        if not input_repo:
            raise ValueError("input_repo cannot be None")
        if not output_repos:
            raise ValueError("output_repos cannot be empty")
        if not src_model:
            raise ValueError("src_model cannot be empty")
        if not dst_model:
            raise ValueError("dst_model cannot be empty")
        if not dst_config:
            raise ValueError("dst_config cannot be empty")
        if not llm_name:
            raise ValueError("llm_name cannot be empty")
        if backend not in ["openai", "gemini", "hf", "vllm", "local"]:
            raise ValueError(f"Invalid backend: {backend}")

        # Validate required dst_config keys
        required_keys = ["filename_desc", "ex_run_cmd", "ex_run_desc",
                        "ex_build_cmd", "ex_build_desc", "build_filename"]
        missing_keys = [key for key in required_keys if key not in dst_config]
        if missing_keys:
            raise ValueError(f"Missing required keys in dst_config: {missing_keys}")


    @staticmethod
    def add_args(parser: 'ArgumentParser'):  # type: ignore # noqa: F821
        """ Add arguments for the naive translation method.
        """
        parser.add_argument("--naive-backend",
                            choices=["openai", "gemini", "hf", "vllm", "local"],
                            default="openai", help="The backend to use for translation.")
        parser.add_argument("--naive-llm-name",
                            type=str, help="The name of the LLM to use for translation.")
        parser.add_argument("--naive-enable-chunking", action="store_true",
                            help="Enable chunking for the naive translation method.")


    @staticmethod
    def parse_args(args: 'Namespace') -> Dict[str, str]:  # type: ignore # noqa: F821
        """ Parse the arguments for the naive translation method.
        """
        return {
            "backend": args.naive_backend,
            "llm_name": args.naive_llm_name,
            "enable_chunking": args.naive_enable_chunking,
            "api_key": args.api_key,
            "api_base_url": args.api_base_url,
            "vllm_environment": args.vllm_environment,
            "vllm_yaml_config": args.vllm_yaml_config,
            "vllm_keepalive_id": args.vllm_keepalive_id,
        }


    def _get_system_prompt(self):
        """ Get and format the system prompt.
        """
        return nc.SYSTEM_TEMPLATE.format(src_model=self._src_model, dst_model=self._dst_model)


    def _get_prompt(self, fname: os.PathLike, chunk: Optional[str] = None) \
            -> Tuple[str, Union[os.PathLike, None]]:
        """ Get and format the prompt for a specific file.

        Args:
            fname: The filename to translate
            chunk: Optional chunk of code to translate instead of full file

        Returns:
            Tuple of (formatted prompt, trigger_rename filename if applicable)
        """
        # Get repository context
        file_tree = self._input_repo.get_file_tree_str()
        all_files_str = self._build_all_files_string(fname, chunk)

        # Get configuration
        prompt_config_src = self._input_repo.get_meta_dict()
        prompt_config_dst = self._dst_config
        exts_str = self._get_extensions_string(prompt_config_dst)

        # Build base prompt
        base_prompt = self._build_base_prompt(
            fname, chunk, file_tree, all_files_str, exts_str, prompt_config_dst
        )

        # Add main file addendum if applicable
        if self._is_main_file(fname, prompt_config_src):
            base_prompt += self._get_main_addendum(prompt_config_dst)

        # Handle build file renaming and addendum
        trigger_rename = self._handle_build_file_renaming(
            fname, prompt_config_src, prompt_config_dst
        )

        if self._is_build_file(fname, prompt_config_src, prompt_config_dst):
            base_prompt += self._get_build_addendum(
                fname, prompt_config_src, prompt_config_dst, exts_str
            )

        return (base_prompt, trigger_rename)


    def _build_all_files_string(self, fname: os.PathLike, chunk: Optional[str] = None) -> str:
        """Build the string containing all files in the repository.

        Args:
            fname: The filename being translated
            chunk: Optional chunk of code

        Returns:
            Formatted string of all files
        """
        all_fpaths = self._input_repo.get_all_filenames(relpaths=True)
        if chunk:
            all_fpaths.remove(fname)

        return "\n\n".join(
            f"{fpath}:\n{self._input_repo.get_file_contents(rel_path=fpath)}"
            for fpath in all_fpaths
        )


    def _get_extensions_string(self, prompt_config_dst: dict) -> str:
        """Get the extensions string for the destination model.

        Args:
            prompt_config_dst: Destination configuration

        Returns:
            Comma-separated list of extensions
        """
        filename_desc = prompt_config_dst["filename_desc"].lower()
        return ", ".join(nc.type_to_ext[filename_desc].values())


    def _build_base_prompt(self, fname: os.PathLike, chunk: Optional[str],
                          file_tree: str, all_files_str: str, exts_str: str,
                          prompt_config_dst: dict) -> str:
        """Build the base prompt template.

        Args:
            fname: The filename to translate
            chunk: Optional chunk of code
            file_tree: Repository file tree string
            all_files_str: All files content string
            exts_str: Extensions string
            prompt_config_dst: Destination configuration

        Returns:
            Formatted base prompt
        """
        if chunk:
            logger.debug("Chunk content:\n%s", chunk)
            return nc.CHUNK_PROMPT_TEMPLATE.format(
                src_model=self._src_model,
                dst_model=self._dst_model,
                file_tree=file_tree,
                all_files=all_files_str,
                filename=fname,
                chunk=chunk,
                exts=exts_str,
                filename_desc=prompt_config_dst["filename_desc"]
            )
        else:
            return nc.PROMPT_TEMPLATE.format(
                src_model=self._src_model,
                dst_model=self._dst_model,
                file_tree=file_tree,
                all_files=all_files_str,
                filename=fname,
                exts=exts_str,
                filename_desc=prompt_config_dst["filename_desc"]
            )


    def _is_main_file(self, fname: os.PathLike, prompt_config_src: dict) -> bool:
        """Check if the file is the main file.

        Args:
            fname: The filename to check
            prompt_config_src: Source configuration

        Returns:
            True if this is the main file
        """
        return fname == prompt_config_src["main_filename"]


    def _get_main_addendum(self, prompt_config_dst: dict) -> str:
        """Get the main file addendum.

        Args:
            prompt_config_dst: Destination configuration

        Returns:
            Formatted main addendum
        """
        return "\n" + nc.MAIN_ADDENDUM.format(
            dst_model=self._dst_model,
            ex_run_cmd=prompt_config_dst["ex_run_cmd"],
            ex_run_desc=prompt_config_dst["ex_run_desc"]
        )


    def _handle_build_file_renaming(self, fname: os.PathLike,
                                   prompt_config_src: dict,
                                   prompt_config_dst: dict) -> Optional[os.PathLike]:
        """Handle build file renaming logic.

        Args:
            fname: The filename to check
            prompt_config_src: Source configuration
            prompt_config_dst: Destination configuration

        Returns:
            Trigger rename filename if applicable, None otherwise
        """
        if prompt_config_dst["build_filename"] != prompt_config_src["build_filename"]:
            return prompt_config_dst["build_filename"]
        return None


    def _is_build_file(self, fname: os.PathLike, prompt_config_src: dict,
                      prompt_config_dst: dict) -> bool:
        """Check if the file is a build file.

        Args:
            fname: The filename to check
            prompt_config_src: Source configuration
            prompt_config_dst: Destination configuration

        Returns:
            True if this is a build file
        """
        key_filename = prompt_config_src["build_filename"]

        # Check if we need to use an extra build file
        if ("extra_build_files" in prompt_config_src
            and prompt_config_dst["build_filename"] in prompt_config_src["extra_build_files"]):
            key_filename = prompt_config_dst["build_filename"]

        return fname == key_filename


    def _get_build_addendum(self, fname: os.PathLike, prompt_config_src: dict,
                           prompt_config_dst: dict, exts_str: str) -> str:
        """Get the build file addendum.

        Args:
            fname: The filename being translated
            prompt_config_src: Source configuration
            prompt_config_dst: Destination configuration
            exts_str: Extensions string

        Returns:
            Formatted build addendum
        """
        key_filename = prompt_config_src["build_filename"]
        if ("extra_build_files" in prompt_config_src
            and prompt_config_dst["build_filename"] in prompt_config_src["extra_build_files"]):
            key_filename = prompt_config_dst["build_filename"]

        return "\n" + nc.BUILD_ADDENDUM.format(
            build_filename=key_filename,
            new_build_filename=prompt_config_dst["build_filename"],
            dst_model=self._dst_model,
            exts=exts_str,
            filename_desc=prompt_config_dst["filename_desc"],
            ex_build_cmd=prompt_config_dst["ex_build_cmd"],
            ex_build_desc=prompt_config_dst["ex_build_desc"]
        )


    def _postprocess(self, output: str) -> Optional[str]:
        """Extract code block from LLM output.

        Args:
            output: Raw output from the LLM

        Returns:
            Extracted code block content or None if not found
        """
        match = CODE_BLOCK_PATTERN.search(output)
        if match is None:
            logger.warning("No code block found in output:\n%s", output)
            return None
        return match.group(1)


    def _update_file_ext(self, fname: os.PathLike,
                         trigger_rename: Optional[os.PathLike] = None) -> os.PathLike:
        """Update filename extension based on the destination model.

        Args:
            fname: Original filename
            trigger_rename: Optional new filename to use

        Returns:
            Updated filename with correct extension
        """
        if trigger_rename:
            fname = trigger_rename

        # Check if file has an extension
        if "." not in str(fname):
            return fname

        name, current_ext = os.path.splitext(fname)

        # Check if the extension is in our mapping
        if current_ext not in nc.ext_to_type:
            return fname

        # Determine extension category
        if self._dst_model == "cuda":
            ext_category = "cuda"
        else:
            ext_category = self._input_repo.get_meta_dict()["filename_desc"]

        # Get new extension
        file_type = nc.ext_to_type[current_ext]
        new_ext = nc.type_to_ext[ext_category.lower()][file_type]

        return name + new_ext


    def _safe_get_columns(self) -> int:
        """Get terminal columns with exception handling.

        Returns:
            Number of terminal columns or default if unavailable
        """
        try:
            return os.get_terminal_size().columns
        except OSError as error:
            if error.errno == 25:  # Inappropriate ioctl for device
                return DEFAULT_TERMINAL_COLS
            raise error


    def _update_interaction_log(self, prompt: str, response: str,
                                reasoning: Optional[str],
                                fpath: os.PathLike,
                                repo_path: os.PathLike) -> None:
        """Write prompt and response to interaction log if logging is enabled.

        Args:
            prompt: The prompt sent to the LLM
            response: The response from the LLM
            reasoning: Optional reasoning from the LLM
            fpath: The file path being translated
            repo_path: The repository path
        """
        if not self._log_interactions:
            return

        log_fpath = os.path.join(repo_path, INTERACTIONS_DIR, f"{fpath}.txt")
        os.makedirs(os.path.dirname(log_fpath), exist_ok=True)

        with open(log_fpath, 'w', encoding="UTF-8") as f:
            f.write("PROMPT:\n")
            f.write(prompt + "\n")
            if reasoning is not None:
                f.write("REASONING:\n")
                f.write(reasoning + "\n")
            f.write("RESPONSE:\n")
            f.write(response + "\n\n")


    def _write_metadata(self, repo_path: Path) -> None:
        """Write experiment metadata to JSON file.

        Args:
            repo_path: The repository path

        Raises:
            OSError: If metadata file cannot be written
            json.JSONEncodeError: If metadata cannot be serialized
        """
        try:
            exp_meta_fpath = os.path.join(repo_path, "..", EXPERIMENT_METADATA_FILE)
            os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)

            exp_meta_dict = self._build_metadata_dict(repo_path)

            with open(exp_meta_fpath, 'w', encoding="UTF-8") as f:
                json.dump(exp_meta_dict, f, indent=4)
            logger.debug("Wrote translation experiment metadata to %s", exp_meta_fpath)
        except (OSError, UnicodeEncodeError, AttributeError, ValueError, TypeError) as e:
            logger.error("Error writing metadata file: %s", e)
            raise


    def _build_metadata_dict(self, repo_path: os.PathLike) -> dict:
        """Build the experiment metadata dictionary.

        Args:
            repo_path: The repository path

        Returns:
            Dictionary containing experiment metadata
        """
        # Extract output number from path (assumes format like "output_123")
        path_parts = str(repo_path).split("/")
        output_number = int(path_parts[-2][7:]) if len(path_parts) > 1 else 0

        return {
            "app": self._input_repo.get_meta_dict()["app"],
            "prompt_strategy": "naive",
            "llm_name": self._llm_name,
            "source_model": self._src_model,
            "dest_model": self._dst_model,
            "output_number": output_number,
            "path": repo_path,
            "inference_stats": self.get_stats()
        }


    def _translate_file(self, fpath: os.PathLike, chunk: Optional[str] = None,
                        chunk_id: int = 0) -> None:
        """Translate a single file.

        Args:
            fpath: The file path to translate
            chunk: Optional chunk of code to translate
            chunk_id: ID of the chunk (0 for full file)
        """
        prompt, trigger_rename = self._get_prompt(fpath, chunk=chunk)
        updated_fname = self._update_file_ext(fpath, trigger_rename=trigger_rename)
        output_fpaths = [Path(rp) / "repo" / updated_fname for rp in self._output_paths]

        if self._dry:
            self._handle_dry_run(prompt, fpath, output_fpaths)
            return

        responses = [self.generate(prompt)[0] for _ in self._output_paths]
        #TODO: parallelize multiple responses

        self._process_responses(responses, prompt, fpath, output_fpaths, chunk, chunk_id)


    def _handle_dry_run(self, prompt: str, fpath: os.PathLike,
                       output_fpaths: List[Path]) -> None:
        """Handle dry run mode.

        Args:
            prompt: The prompt that would be sent
            fpath: The file path being translated
            output_fpaths: List of output file paths
        """
        logger.debug("Dry-run prompt:\n%s", prompt)
        logger.info("Skipped translation of %s to %s..%s for dry run.",
                    fpath, output_fpaths[0], output_fpaths[-1])


    def _process_responses(self, responses: List[GenericResponse], prompt: str,
                          fpath: os.PathLike, output_fpaths: List[Path],
                          chunk: Optional[str], chunk_id: int) -> None:
        """Process LLM responses and write to files.

        Args:
            responses: List of responses from the LLM
            prompt: The original prompt
            fpath: The file path being translated
            output_fpaths: List of output file paths
            chunk: Optional chunk being translated
            chunk_id: ID of the chunk
        """
        for i, response in enumerate(responses):
            output_fpath, repo_path = output_fpaths[i], self._output_paths[i]

            # Log interaction
            self._update_interaction_log(
                prompt, response.response, response.reasoning, fpath, repo_path
            )

            # Process output
            output = self._postprocess(response.response)
            if output is None:
                logger.warning("Failed to translate %s to %s.", fpath, output_fpath)
                continue

            # Write to file
            self._write_translated_file(output_fpath, output, chunk_id)
            self._print_translation_status(fpath, output_fpath, chunk, chunk_id)


    def _write_translated_file(self, output_fpath: os.PathLike,
                              output: str, chunk_id: int) -> None:
        """Write translated content to file.

        Args:
            output_fpath: Path to write the file
            output: Translated content
            chunk_id: ID of the chunk (0 for full file)

        Raises:
            OSError: If file cannot be written
        """
        try:
            os.makedirs(os.path.dirname(output_fpath), exist_ok=True)
            open_mode = "w" if chunk_id == 0 else "a"

            with open(output_fpath, open_mode, encoding="UTF-8") as f:
                f.write(output)
        except OSError as e:
            logger.error("Error writing file %s: %s", output_fpath, e)
            raise


    def _print_translation_status(self, fpath: os.PathLike, output_fpath: os.PathLike,
                                 chunk: Optional[str], chunk_id: int) -> None:
        """Log translation status message."""
        if chunk:
            logger.info("Translated %s to %s (chunk %d).", fpath, output_fpath, chunk_id)
        else:
            logger.info("Translated %s to %s.", fpath, output_fpath)


    def translate(self) -> None:
        """Translate the entire repository.

        This is the main entry point for translation. It processes all files
        in the repository, handling chunking if enabled, and generates
        experiment metadata.
        """
        all_files = [Path(f) for f in self._input_repo.get_all_filenames(relpaths=True)]
        num_translations = len(self._output_paths)
        repo_paths = [Path(self._output_paths[i]) / "repo"
                      for i in range(num_translations)]
        max_cols = self._safe_get_columns()

        self._print_translation_start(num_translations, repo_paths, all_files)

        # Translate all files
        for fpath in alive_it(all_files,
                              title="Translating files",
                              max_cols=max_cols,
                              disable=self._hide_progress):
            self._translate_single_file(fpath)

        # Write experiment metadata
        self._write_all_metadata(repo_paths)


    def _print_translation_start(self, num_translations: int,
                                repo_paths: List[Path],
                                all_files: List[Path]) -> None:
        """Print translation start information.

        Args:
            num_translations: Number of translations to perform
            repo_paths: List of repository paths
            all_files: List of files to translate
        """
        logger.info("Beginning %d batched translation(s) starting from %s using %s with NaiveTranslator.",
                    num_translations, repo_paths[0], self._llm_name)
        logger.debug("Files to translate: %s", all_files)


    def _translate_single_file(self, fpath: os.PathLike) -> None:
        """Translate a single file, handling chunking if enabled.

        Args:
            fpath: The file path to translate
        """
        if not self._chunk_agent:
            self._translate_file(fpath)
            return

        # Handle chunking
        logger.debug("Chunking file %s...", fpath)
        source_code = self._input_repo.get_file_contents(rel_path=fpath)
        chunks = self._chunk_agent.chunk_file(source_code)
        logger.debug("Chunked %s into %d chunk(s).", fpath, len(chunks))

        if len(chunks) > 1:
            for i, chunk in enumerate(chunks):
                self._translate_file(fpath, chunk=chunk, chunk_id=i)
        else:
            # If only one chunk, translate as normal file
            self._translate_file(fpath)


    def _write_all_metadata(self, repo_paths: List[Path]) -> None:
        """Write experiment metadata for all repositories.

        Args:
            repo_paths: List of repository paths
        """
        for repo_path in repo_paths:
            self._write_metadata(repo_path)
