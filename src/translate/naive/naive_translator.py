""" Naively translate file-by-file from one repository to the other. Include
    as much context in the prompt as possible.
"""
# std imports
import os
import sys
import re
import json
from abc import ABC, abstractmethod

# tpl imports
from alive_progress import alive_it

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from repo import Repo


class NaiveTranslator(Translator):

    SYSTEM_TEMPLATE: str = """You are a helpful coding assistant.
You are helping a software developer translate a codebase from the {src_model} execution model to the {dst_model} execution model."""

    PROMPT_TEMPLATE: str = """Below is a codebase written in the {src_model} execution model. We are translating it to the {dst_model} execution model.
Here is the file tree of the entire repository:

{file_tree}

Here is the code for each file in the codebase:

{all_files}

Translate the {filename} file to the {dst_model} execution model. Output each translated code file (source files, header files, and Makefiles) in one code block.
"""

    MAIN_ADDENDUM: str = """This file includes the main function. Please ensure the command line interface after translation still works as expected, so that, for example, `{ex_run_cmd}` still works to run the code with {ex_run_desc}.
"""

    MAKEFILE_ADDENDUM: str = """This file is a Makefile. Please output a Makfile converted to compile this code as a {dst_model} code. Assume reasonable filenames for a {filename_desc} code, and that the user will compile this code using, for example, `{ex_build_cmd}` to build the code for {ex_build_desc}.
"""

    def get_system_prompt(self):
        return self.SYSTEM_TEMPLATE.format(src_model=self._src_model, dst_model=self._dst_model)

    def get_prompt(self, fname: str):
        file_tree = self._input_repo.get_file_tree_str()
        all_fpaths = self._input_repo.get_all_filenames(relpaths=True)
        all_files_str = "\n\n".join(map(lambda fpath: fpath + ":\n" + self._input_repo.get_file_contents(rel_path=fpath), all_fpaths))
        base_prompt = self.PROMPT_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model,
            file_tree=file_tree,
            all_files=all_files_str,
            filename=fname
        )
        if fname == self._prompt_config["main_filename"]:
            base_prompt += ("\n" + MAIN_ADDENDUM.format(
                dst_model=self._dst_model,
                ex_run_cmd=self._prompt_config["ex_run_cmd"],
                ex_run_desc=self._prompt_config["ex_run_desc"]))
        if fname == self._prompt_config["build_filename"]:
            base_prompt += ("\n" + MAKEFILE_ADDENDUM.format(
                dst_model=self._dst_model,
                filename_desc=self._prompt_config["filename_desc"],
                ex_build_cmd=self._prompt_config["ex_build_cmd"],
                ex_build_desc=self._prompt_config["ex_build_desc"]))
        return base_prompt

    CODE_BLOCK_PATTERN = re.compile(r"```(?:\w+)?\n(.*?)\n```", re.DOTALL)

    def _postprocess(self, output: str) -> str:
        # make sure there's only one codeblock and extract it
        match = self.CODE_BLOCK_PATTERN.search(output)
        if match is None:
            raise ValueError("No code block found in output.")
        return match.group(1)

    def update_output_file_extension(self, fname: str) -> str:
        """ Return the filename with updated extension based on the destination model """
        # Dicts of file extension mappings
        ext_to_type = {
            ".cu": "code",
            ".cuh": "header",
            ".cpp": "code",
            ".h": "header",
            ".hpp": "header",
            ".c": "code",
            ".cc": "code",
            ".cxx": "code",
            ".hh": "header",
            ".hxx": "header",
            ".c++": "code",
            ".h++": "header"
        }
        type_to_ext = {
            "cuda": {"code": ".cu", "header": ".cuh"},
            "C++": {"code": ".cpp", "header": ".hpp"},
            "C": {"code": ".c", "header": ".h"}
        }

        # Check if file has an extension
        if "." in fname:
            name, current_ext = os.path.splitext(fname)

            # Check if the extension is in the dict
            if current_ext in ext_to_type:
                if self._dst_model == "cuda":
                    ext_category = self._dst_model
                else:
                    ext_category = self._input_repo.get_meta_dict()["filename_desc"]
                return name + type_to_ext[ext_category][ext_to_type[current_ext]]
        return fname

    @abstractmethod
    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        pass

    def translate(self, dry: bool = False, log_interactions: bool = False):
        system_prompt = self.get_system_prompt()
        all_files = self._input_repo.get_all_filenames(relpaths=True)
        repo_fpath = os.path.join(self._output_fpath, f"output-{self._output_id}", "repo")

        for fpath in alive_it(all_files, title="Translating files"):
            prompt = self.get_prompt(fpath)

            output_fpath = os.path.join(repo_fpath, self.update_output_file_extension(fpath))

            if dry:
                print(prompt)
                print(f"Skipped translation of {fpath} to {output_fpath} for dry run.")
                continue

            output = self._get_translation(system_prompt, prompt)

            if log_interactions:
                log_fpath = os.path.join(self._output_fpath, f"output-{self._output_id}", "interactions", f"{fpath}.txt")
                os.makedirs(os.path.dirname(log_fpath), exist_ok=True)
                with open(log_fpath, 'w') as f:
                    f.write(output)
                print(f"Logged interaction to {log_fpath}")

            output = self._postprocess(output)

            # make parent dirs if necessary
            os.makedirs(os.path.dirname(output_fpath), exist_ok=True)

            with open(output_fpath, 'w') as f:
                f.write(output)
            print(f"Translated {fpath} to {output_fpath}")

        # Write experiment_metadata.json
        exp_meta_fpath = os.path.join(self._output_fpath, f"output-{self._output_id}", "experiment_metadata.json")
        os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)
        with open(exp_meta_fpath, 'w') as f:
            exp_meta_dict = {
                "app": self._app_name,
                "prompt_strategy": "naive",
                "llm_name": self._llm_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": self._output_id,
                "path": repo_fpath
            }
            json.dump(exp_meta_dict, f, indent=4)
        print(f"Wrote translation experiment metadata to {exp_meta_fpath}")
