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

Translate the {filename} file to the {dst_model} execution model. Output the translated file in one code block. Assume {exts} filenames whenever referring to other files as this will be a {filename_desc} code.
"""

    MAIN_ADDENDUM: str = """This file includes the main function. Please ensure the command line interface after translation still works as expected, so that, for example, `{ex_run_cmd}` still works to run the code with {ex_run_desc}.
"""

    BUILD_ADDENDUM: str = """This file is a Makefile. Please output a {new_build_file} to compile this code as a {dst_model} code. Assume {exts} filenames as this will be a {filename_desc} code, and that the user will compile this code using, for example, `{ex_build_cmd}` to build the code for {ex_build_desc}.
"""

    # Dicts of file extension mappings
    _ext_to_type: dict = {
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

    _type_to_ext: dict = {
        "cuda": {"code": ".cu", "header": ".cuh"},
        "c++": {"code": ".cpp", "header": ".hpp"},
        "c": {"code": ".c", "header": ".h"}
    }

    def get_system_prompt(self):
        return self.SYSTEM_TEMPLATE.format(src_model=self._src_model, dst_model=self._dst_model)

    def get_prompt(self, fname: str):
        file_tree = self._input_repo.get_file_tree_str()
        all_fpaths = self._input_repo.get_all_filenames(relpaths=True)
        all_files_str = "\n\n".join(
            map(lambda fpath:
                fpath + ":\n"
                + self._input_repo.get_file_contents(rel_path=fpath),
                all_fpaths))

        exts_str = ", ".join(v for v in self._type_to_ext[self._prompt_config_dst["filename_desc"].lower()].values())
        base_prompt = self.PROMPT_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model,
            file_tree=file_tree,
            all_files=all_files_str,
            filename=fname,
            exts=exts_str,
            filename_desc=self._prompt_config_dst["filename_desc"])

        if fname == self._prompt_config_src["main_filename"]:
            base_prompt += ("\n" + self.MAIN_ADDENDUM.format(
                dst_model=self._dst_model,
                ex_run_cmd=self._prompt_config_dst["ex_run_cmd"],
                ex_run_desc=self._prompt_config_dst["ex_run_desc"]))

        trigger_rename = None
        key_filename = self._prompt_config_src["build_filename"]
        if self._prompt_config_dst["build_filename"] != self._prompt_config_src["build_filename"]:
            trigger_rename = self._prompt_config_dst["build_filename"]

            # If the repo being translated already has the build file that we want
            # to use, but it's not the default build file, then we should trigger
            # the build addendum for that extra build file rather than the default.
            if ("extra_build_files" in self._prompt_config_src
                and self._prompt_config_dst["build_filename"] in self._prompt_config_src["extra_build_files"]):
                key_filename = self._prompt_config_dst["build_filename"]

        if fname == key_filename:
            base_prompt += ("\n" + self.BUILD_ADDENDUM.format(
                new_build_file=self._prompt_config_dst["build_filename"],
                dst_model=self._dst_model,
                exts=exts_str,
                filename_desc=self._prompt_config_dst["filename_desc"],
                ex_build_cmd=self._prompt_config_dst["ex_build_cmd"],
                ex_build_desc=self._prompt_config_dst["ex_build_desc"]))
        else:
            trigger_rename = None

        return (base_prompt, trigger_rename)

    CODE_BLOCK_PATTERN = re.compile(r"```(?:[+\w]+)?\n(.*?)\n```", re.DOTALL)

    def _postprocess(self, output: str) -> str:
        # make sure there's only one codeblock and extract it
        match = self.CODE_BLOCK_PATTERN.search(output)
        if match is None:
            raise ValueError("No code block found in output.")
        return match.group(1)

    def update_output_file_extension(self, fname: str, trigger_rename: str = None) -> str:
        """ Return the filename with updated extension based on the destination model """

        if trigger_rename:
            fname = trigger_rename

        # Check if file has an extension
        if "." in fname:
            name, current_ext = os.path.splitext(fname)

            # Check if the extension is in the dict
            if current_ext in self._ext_to_type:
                if self._dst_model == "cuda":
                    ext_category = "cuda"
                else:
                    ext_category = self._prompt_config_dst["filename_desc"].lower()
                return name + self._type_to_ext[ext_category][self._ext_to_type[current_ext]]

        return fname

    @abstractmethod
    def _get_translation(self, system_prompt: str, prompt: str) -> str:
        pass

    def translate(self, dry: bool = False, log_interactions: bool = False, hide_progress: bool = False):
        system_prompt = self.get_system_prompt()
        all_files = self._input_repo.get_all_filenames(relpaths=True)
        repo_fpath = os.path.join(self._output_fpath, "repo")

        try:
            max_cols = os.get_terminal_size().columns
        except OSError as error:
            if error.errno == 25:
                max_cols = 80
            else:
                raise error
        for fpath in alive_it(all_files, title="Translating files", max_cols=max_cols, disable=hide_progress):
            prompt, trigger_rename = self.get_prompt(fpath)

            output_fpath = os.path.join(repo_fpath, self.update_output_file_extension(fpath, trigger_rename=trigger_rename))

            if dry:
                print(prompt)
                print(f"Skipped translation of {fpath} to {output_fpath} for dry run.")
                continue

            output = self._get_translation(system_prompt, prompt)

            if log_interactions:
                log_fpath = os.path.join(self._output_fpath, "interactions", f"{fpath}.txt")
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
        exp_meta_fpath = os.path.join(self._output_fpath, "experiment_metadata.json")
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
