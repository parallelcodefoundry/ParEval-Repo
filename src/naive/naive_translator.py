""" Naively translate file-by-file from one repository to the other. Include
    as much context in the prompt as possible.
"""
# std imports
import os
import sys
import re

# tpl imports
from alive_progress import alive_it
from openai import OpenAI

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator


class NaiveTranslator(Translator):

    SYSTEM_TEMPLATE: str = """You are a helpful coding assistant.
You are helping a software developer translate a codebase from the {src_model} execution model to the {dst_model} execution model."""

    PROMPT_TEMPLATE: str = """Below is a codebase written in the {src_model} execution model. We are translating it to the {dst_model} execution model.
Here is the file tree of the entire repository:

{file_tree}

Here is the code for each file in the codebase:

{all_files}

Translate the {filename} file to the {dst_model} execution model. Output the translated code in one code block.
"""

    def __init__(self, input_repo, output_repo, src_model, dst_model):
        super().__init__(input_repo, output_repo, src_model, dst_model)

        self._api_client = OpenAI()

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


    def translate(self, dry: bool = False):
        system_prompt = self.get_system_prompt()
        all_files = self._input_repo.get_all_filenames(relpaths=True)

        for fpath in alive_it(all_files, title="Translating files"):
            prompt = self.get_prompt(fpath)
            
            if dry:
                print(prompt)
                continue

            completion = self._api_client.chat.completions.create(
                model="gpt-4-turbo",
                messages=[
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": prompt}
                ],
                max_tokens=4096,
                temperature=0.2,
                top_p=0.96,
                stream=False,
                n=1
            )
            output = completion.choices[0].message.content
            output = self._postprocess(output)
            
            output_fpath = os.path.join(self._output_fpath, fpath)

            # make parent dirs if necessary
            os.makedirs(os.path.dirname(output_fpath), exist_ok=True)

            with open(output_fpath, 'w') as f:
                f.write(output)
            print(f"Translated {fpath} to {output_fpath}")

