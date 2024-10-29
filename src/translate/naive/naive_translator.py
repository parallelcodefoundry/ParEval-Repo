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
        return self.PROMPT_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model,
            file_tree=file_tree,
            all_files=all_files_str,
            filename=fname
        )

    CODE_BLOCK_PATTERN = re.compile(r"```(?:\w+)?\n(.*?)\n```", re.DOTALL)

    def _postprocess(self, output: str) -> str:
        # make sure there's only one codeblock and extract it
        match = self.CODE_BLOCK_PATTERN.search(output)
        if match is None:
            raise ValueError("No code block found in output.")
        return match.group(1)

    @abstractmethod
    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        pass

    def translate(self, dry: bool = False):
        system_prompt = self.get_system_prompt()
        all_files = self._input_repo.get_all_filenames(relpaths=True)
        repo_fpath = os.path.join(self._output_fpath, f"output-{self._output_id}", "repo")

        for fpath in alive_it(all_files, title="Translating files"):
            prompt = self.get_prompt(fpath)

            if dry:
                print(prompt)
                continue

            output = self._get_translation(system_prompt, prompt)

            output = self._postprocess(output)

            output_fpath = os.path.join(repo_fpath, fpath)

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
