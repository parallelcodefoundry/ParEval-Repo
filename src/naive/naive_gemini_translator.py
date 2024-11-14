"""Naively translates a repository file-by-file from one execution model to
   another execution model using Gemini
"""
# std imports
import os
import re
import sys
import json

# tpl imports
import google.generativeai as genai
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))

# local imports
from translator import Translator
from repo import Repo


class NaiveGeminiTranslator(Translator):
    SYSTEM_TEMPLATE: str = """You are a helpful coding assistant.
You are helping a software developer translate a codebase from the {src_model} execution model to the {dst_model} execution model."""

    PROMPT_TEMPLATE: str = """Below is a codebase written in the {src_model} execution model. We are translating it to the {dst_model} execution model.
Here is the file tree of the entire repository:

{file_tree}

Here is the code for each file in the codebase:

{all_files}

Translate the {filename} file to the {dst_model} execution model. Output each translated code file (source files, header files, and Makefiles) in one code block.
"""

    def __init__(self, input_repo: Repo, output_repo: os.PathLike, src_model: str, dst_model: str, output_id: int, app_name: str):
        super().__init__(input_repo, output_repo, src_model, dst_model, output_id, app_name)
        genai.configure(api_key=os.environ["GEMINI_API_KEY"])
        self._model = genai.GenerativeModel("gemini-1.5-flash")

    def get_system_prompt(self):
        return self.SYSTEM_TEMPLATE.format(src_model=self._src_model, dst_model=self._dst_model)

    def get_prompt(self, fname: str):
        file_tree = self._input_repo.get_file_tree_str()
        all_fpaths = self._input_repo.get_all_filenames(relpaths=True)
        all_files_str = "\n\n".join(
            map(lambda fpath: fpath + ":\n" + self._input_repo.get_file_contents(rel_path=fpath), all_fpaths)
        )
        return self.PROMPT_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model,
            file_tree=file_tree,
            all_files=all_files_str,
            filename=fname
        )

    CODE_BLOCK_PATTERN = re.compile(r"```(?:\w+)?\n(.*?)\n```", re.DOTALL)

    def _postprocess(self, output: str) -> str:
        match = self.CODE_BLOCK_PATTERN.search(output)
        if match is None:
            raise ValueError("No code block found in output.")
        return match.group(1)

    def translate(self, dry: bool = False):
        system_prompt = self.get_system_prompt()
        all_files = self._input_repo.get_all_filenames(relpaths=True)
        repo_fpath = os.path.join(self._output_fpath, f"output-{self._output_id}", "repo")

        for fpath in all_files:
            prompt = self.get_prompt(fpath)

            if dry:
                print(prompt)
                continue

            response = self._model.generate_content(prompt)
            output = response.text
            output = self._postprocess(output)

            output_fpath = os.path.join(repo_fpath, fpath)

            # make parent dirs if necessary
            os.makedirs(os.path.dirname(output_fpath), exist_ok=True)

            with open(output_fpath, 'w') as f:
                f.write(output)
            print(f"Translated {fpath} to {output_fpath}")

        # Write meta.json
        meta_fpath = os.path.join(self._output_fpath, f"output-{self._output_id}", "meta.json")
        os.makedirs(os.path.dirname(meta_fpath), exist_ok=True)
        with open(meta_fpath, 'w') as f:
            meta_dict = {
                "app": self._app_name,
                "prompt_strategy": "naive",
                "llm_name": "gemini",
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": self._output_id,
                "path": repo_fpath
            }
            json.dump(meta_dict, f, indent=4)
        print(f"Wrote translation metadata to {meta_fpath}")
