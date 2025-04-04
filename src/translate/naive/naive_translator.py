""" Naively translate file-by-file from one repository to the other. Include
    as much context in the prompt as possible.
"""
# std imports
import os
import sys
import re
import json
from abc import abstractmethod
from typing import Dict, Tuple, Union, Literal, List, Optional

# tpl imports
from alive_progress import alive_it

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from generator_mixin import GeneratorMixin, GenericResponse
from repo import Repo
from naive import naive_constants as nc
from restate.chunk_agent import ChunkFileAgent

class NaiveTranslator(Translator, GeneratorMixin):
    """ Class to naively translate file-by-file from one repository to the other.
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
            hide_progress: bool = False
    ):
        super().__init__(input_repo, output_repos, src_model, dst_model,
                         dst_config, log_interactions, dry, hide_progress)

        GeneratorMixin.__init__(self, backend, llm_name,
                                system_prompt=self._get_system_prompt())

        if enable_chunking:
            self._chunk_agent = ChunkFileAgent(self, max_tokens=1024)


    @staticmethod
    def add_args(parser: 'ArgumentParser'):  # type: ignore # noqa: F821
        """ Add arguments for the naive translation method.
        """
        parser.add_argument("--naive-backend",
                            choices=["openai", "gemini", "hf", "vllm", "local"],
                            default="openai", help="The backend to use for translation.")
        parser.add_argument("--naive-llm-name",
                            type=str, help="The name of the LLM to use for translation.")
        parser.add_argument("--enable-chunking", action="store_true",
                            help="Enable chunking for the naive translation method.")


    @staticmethod
    def parse_args(args: 'Namespace') -> Dict[str, str]:  # type: ignore # noqa: F821
        """ Parse the arguments for the naive translation method.
        """
        return {
            "backend": args.naive_backend,
            "llm_name": args.naive_llm_name,
            "enable_chunking": args.enable_chunking,
        }


    def _get_system_prompt(self):
        """ Get and format the system prompt.
        """
        return nc.SYSTEM_TEMPLATE.format(src_model=self._src_model, dst_model=self._dst_model)


    def _get_prompt(self, fname: os.PathLike, chunk: Optional[str] = None) \
            -> Tuple[str, Union[str, None]]:
        """ Get and format the prompt for a specific file.
        """
        file_tree = self._input_repo.get_file_tree_str()
        all_fpaths = self._input_repo.get_all_filenames(relpaths=True)
        if chunk:
            all_fpaths.remove(fname)
        all_files_str = "\n\n".join(
            map(lambda fpath:
                str(fpath) + ":\n"
                + self._input_repo.get_file_contents(rel_path=fpath),
                all_fpaths))
        prompt_config_src = self._input_repo.get_meta_dict()
        prompt_config_dst = self._dst_config

        exts_str = ", ".join(v for v in nc.type_to_ext[prompt_config_dst["filename_desc"].lower()].values())
        if chunk:
            print(chunk)
            base_prompt = nc.CHUNK_PROMPT_TEMPLATE.format(
                src_model=self._src_model,
                dst_model=self._dst_model,
                file_tree=file_tree,
                all_files=all_files_str,
                filename=fname,
                chunk=chunk,
                exts=exts_str,
                filename_desc=prompt_config_dst["filename_desc"])
        else:
            base_prompt = nc.PROMPT_TEMPLATE.format(
                src_model=self._src_model,
                dst_model=self._dst_model,
                file_tree=file_tree,
                all_files=all_files_str,
                filename=fname,
                exts=exts_str,
                filename_desc=prompt_config_dst["filename_desc"])

        if fname == prompt_config_src["main_filename"]:
            base_prompt += ("\n" + nc.MAIN_ADDENDUM.format(
                dst_model=self._dst_model,
                ex_run_cmd=prompt_config_dst["ex_run_cmd"],
                ex_run_desc=prompt_config_dst["ex_run_desc"]))

        trigger_rename = None
        key_filename = prompt_config_src["build_filename"]
        if prompt_config_dst["build_filename"] != prompt_config_src["build_filename"]:
            trigger_rename = prompt_config_dst["build_filename"]

            # If the repo being translated already has the build file that we want
            # to use, but it's not the default build file, then we should trigger
            # the build addendum for that extra build file rather than the default.
            if ("extra_build_files" in prompt_config_src
                and prompt_config_dst["build_filename"] in prompt_config_src["extra_build_files"]):
                key_filename = prompt_config_dst["build_filename"]

        if fname == key_filename:
            base_prompt += ("\n" + nc.BUILD_ADDENDUM.format(
                build_filename=key_filename,
                new_build_filename=prompt_config_dst["build_filename"],
                dst_model=self._dst_model,
                exts=exts_str,
                filename_desc=prompt_config_dst["filename_desc"],
                ex_build_cmd=prompt_config_dst["ex_build_cmd"],
                ex_build_desc=prompt_config_dst["ex_build_desc"]))
        else:
            trigger_rename = None

        return (base_prompt, trigger_rename)


    def _postprocess(self, output: str) -> str:
        """ make sure there's only one codeblock and extract it
        """
        CODE_BLOCK_PATTERN = re.compile(r"```(?:[+\w]+)?\n(.*?)\n```", re.DOTALL)
        match = CODE_BLOCK_PATTERN.search(output)
        if match is None:
            raise ValueError(f"No code block found in output:\n{output}")
        return match.group(1)


    def _update_output_file_extension(self, fname: os.PathLike,
                                      trigger_rename: Optional[os.PathLike] = None) \
                                      -> os.PathLike:
        """ Return the filename with updated extension based on the destination model.
        """

        if trigger_rename:
            fname = trigger_rename

        # Check if file has an extension
        if "." in str(fname):
            name, current_ext = os.path.splitext(fname)

            # Check if the extension is in the dict
            if current_ext in nc.ext_to_type:
                if self._dst_model == "cuda":
                    ext_category = "cuda"
                else:
                    ext_category = self._input_repo.get_meta_dict()["filename_desc"]
                return name + nc.type_to_ext[ext_category.lower()][nc.ext_to_type[current_ext]]
        return fname


    def _safe_get_columns(self) -> int:
        """ Return get_terminal_size with exception handling.
        """
        try:
            max_cols = os.get_terminal_size().columns
        except OSError as error:
            if error.errno == 25:
                max_cols = 80
            else:
                raise error
        return max_cols


    def _update_interaction_log(self, prompt: str, response: str,
                                reasoning: Union[str, None],
                                fpath: os.PathLike,
                                repo_path: os.PathLike):
        """ Write prompt and response to interaction log for the given filename
            if logging is enabled.
        """
        if self._log_interactions:
            log_fpath = os.path.join(repo_path, "interactions", f"{fpath}.txt")
            os.makedirs(os.path.dirname(log_fpath), exist_ok=True)
            with open(log_fpath, 'w', encoding="UTF-8") as f:
                f.write("PROMPT:\n")
                f.write(prompt + "\n")
                if reasoning is not None:
                    f.write("REASONING:\n")
                    f.write(reasoning + "\n")
                f.write("RESPONSE:\n")
                f.write(response + "\n\n")


    def _write_metadata(self, repo_path: os.PathLike):
        """ Write out experiment_metadata.json adjacent to repo path.
        """
        exp_meta_fpath = os.path.join(repo_path, "..", "experiment_metadata.json")
        os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)

        with open(exp_meta_fpath, 'w', encoding="UTF-8") as f:
            exp_meta_dict = {
                "app": self._input_repo.get_meta_dict()["app"],
                "prompt_strategy": "naive",
                "llm_name": self._llm_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": int(str(repo_path).split("/")[-2][7:]),
                "path": repo_path,
                "inference_stats": self.get_stats()
            }
            json.dump(exp_meta_dict, f, indent=4)
        print(f"Wrote translation experiment metadata to {exp_meta_fpath}")


    def _translate_file(self, fpath: os.PathLike, chunk: Optional[str] = None,
                        chunk_id: Optional[int] = 0):
        """ Translate a single file.
        """
        prompt, trigger_rename = self._get_prompt(fpath, chunk=chunk)
        updated_fname = self._update_output_file_extension(fpath,
                                                           trigger_rename=trigger_rename)
        output_fpaths = [os.path.join(rp, updated_fname) for rp in self._output_paths]

        if self._dry:
            print(prompt)
            print(f"Skipped translation of {fpath} to " +
                  f"{output_fpaths[0]}..{output_fpaths[-1]} for dry run.")
            return

        responses = self.generate(prompt, temperature=0.2, top_p=0.95,
                                  n=len(self._output_paths))
        for i, response in enumerate(responses):
            output_fpath, repo_path = output_fpaths[i], self._output_paths[i]
            raw_output, reasoning = response.response, response.reasoning
            self._update_interaction_log(prompt, raw_output, reasoning,
                                         fpath, repo_path)
            output = self._postprocess(raw_output)

            os.makedirs(os.path.dirname(output_fpath), exist_ok=True)
            open_mode = "w" if chunk_id == 0 else "a"
            with open(output_fpath, open_mode, encoding="UTF-8") as f:
                f.write(output)
            print(f"Translated {fpath} to {output_fpath}", end="")
            if chunk:
                print(f" (chunk {chunk_id})")
            else:
                print("")


    # override
    def translate(self):
        """ Translate the entire repository.
        """
        all_files = self._input_repo.get_all_filenames(relpaths=True)
        num_translations = len(self._output_paths)
        repo_paths = [os.path.join(self._output_paths[i], f"output-{i}") \
                       for i in range(num_translations)]
        max_cols = self._safe_get_columns()

        print(f"Beginning {num_translations} batched translation(s) " +
              f"starting from {repo_paths[0]} using {self._llm_name} with NaiveTranslator.")
        print(f"Files to translate: {all_files}")

        # loop over all files and translate
        for fpath in alive_it(all_files,
                              title="Translating files",
                              max_cols=max_cols,
                              disable=self._hide_progress):
            if self._chunk_agent:
                print(f"Chunking file {fpath}...")
                source_code = self._input_repo.get_file_contents(rel_path=fpath)
                chunks = self._chunk_agent.chunk_file(source_code)
                print(f"Chunked {fpath} into {len(chunks)} chunks.")
                if len(chunks) > 1:
                    for i, chunk in enumerate(chunks):
                        self._translate_file(fpath, chunk=chunk, chunk_id=i)
                    continue
            self._translate_file(fpath)

        # dump experiment metadata files
        for repo_path in repo_paths:
            self._write_metadata(repo_path)
