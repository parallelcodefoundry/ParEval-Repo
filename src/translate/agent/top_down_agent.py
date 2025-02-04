""" An agent-based approach to translating entire code repositories from one
    execution model to another (i.e. CUDA to OpenMP or OpenMP to MPI). This
    approach is based on the top-down translation method. The agent will first
    determine the dependency tree of source files and then translate them
    starting from the root. Intermediate helper agents are used to split up
    long files, incorporate context from further up the tree, and to determine
    filenames and other metadata.

    author: Daniel Nichols
    date: November 2024
"""
# std imports
import os
import sys
import json
from typing import Dict, Literal

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from agent.dependency_agent import DependencyAgent, FileNode, FileType
from agent.chunk_agent import ChunkFileAgent
from agent.context_agent import ContextAgent
from agent import agent_constants as ac
from generator_mixin import GeneratorMixin
from repo import Repo


class TopDownAgentTranslator(Translator, GeneratorMixin):
    """ A class to translate an entire repository using the top-down agent method.
    """

    _dependency_agent: DependencyAgent
    _chunk_file_agent: ChunkFileAgent
    _context_agent: ContextAgent
    _system_prompt: str

    def __init__(
            self,
            input_repo: Repo,
            output_repo: os.PathLike,
            src_model: str,
            dst_model: str,
            dst_config: dict,
            llm_name: str,
            backend: Literal["openai", "gemini", "hf", "local"] = "openai",
            log_interactions: bool = False,
            dry: bool = False,
            hide_progress: bool = False
    ):
        super().__init__(input_repo, output_repo, src_model, dst_model,
                         dst_config, log_interactions, dry, hide_progress)

        self._system_prompt = ac.SYSTEM_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model)

        GeneratorMixin.__init__(self, backend, llm_name)

        interactions_path = None
        if self._log_interactions:
            interactions_path = os.path.join(self._output_fpath, "interactions.txt")
        self._dependency_agent = DependencyAgent(generator=self,
                                                 interactions_path=interactions_path)
        self._chunk_file_agent = ChunkFileAgent(generator=self,
                                                interactions_path=interactions_path)
        self._context_agent = ContextAgent(generator=self,
                                           interactions_path=interactions_path,
                                           output_fpath=self._output_fpath)

    @staticmethod
    def add_args(parser: 'ArgumentParser'): # type: ignore # noqa: F821
        """ Add arguments for the top-down agent translation method.
        """
        parser.add_argument("--agent-backend",
                            choices=["openai", "gemini", "hf", "local"],
                            default="openai", help="The backend to use for translation.")
        parser.add_argument("--agent-llm-name",
                            type=str, help="The name of the LLM to use for translation.")

    @staticmethod
    def parse_args(args: 'Namespace') -> Dict[str, str]: # type: ignore # noqa: F821
        """ Parse the arguments for the top-down agent translation method.
        """
        return {
            "backend": args.agent_backend,
            "llm_name": args.agent_llm_name
        }


    def _read_file(self, rel_path: str) -> str:
        """ Read the contents of a file from the input repository.
        """
        input_file_path = os.path.join(self._input_repo.path, rel_path)
        with open(input_file_path, 'r', encoding="UTF-8") as f:
            return f.read()


    def _write_file(self, rel_path: str, contents: str):
        """ Write the contents to a file in the output repository.
        """
        output_file_path = os.path.join(self._output_fpath, "repo", rel_path)

        # make parent dirs if necessary
        os.makedirs(os.path.dirname(output_file_path), exist_ok=True)

        with open(output_file_path, 'w', encoding="UTF-8") as f:
            f.write(contents)
        print(f"Wrote file {output_file_path}")

    def _write_metadata(self, repo_fpath: os.PathLike):
        """ Write out experiment_metadata.json adjacent to repo path.
        """
        exp_meta_fpath = os.path.join(self._output_fpath, "experiment_metadata.json")
        os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)
        with open(exp_meta_fpath, 'w', encoding="UTF-8") as f:
            exp_meta_dict = {
                "app": self._input_repo.get_meta_dict()["app"],
                "prompt_strategy": "naive",
                "llm_name": self._llm_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": int(repo_fpath.split("/")[-2][7:]),
                # todo: consider dropping the output_number field
                "path": repo_fpath
            }
            json.dump(exp_meta_dict, f, indent=4)
        print(f"Wrote translation experiment metadata to {exp_meta_fpath}")

    # override
    def translate(self):
        """ Use the top-down method to translate the entire repository.
        """
        print(f"Constructing dependency graph on {self._input_repo.path}...")
        dep_tree = self._dependency_agent.construct_dependency_graph(self._input_repo.path)

        # walk down the tree and translate each file
        for node in dep_tree:
            self._translate_node(node)

        # dump experiment metadata
        self._write_metadata(os.path.join(self._output_fpath, "repo"))

    def _translate_node(self, node: FileNode):
        """ Translate a single file node using context from its parents.
        """
        print(f"Translating file {node.rel_path}...")

        # get the source code from the file
        source_code = self._read_file(node.rel_path)

        # get the context from the parent nodes
        context = self._context_agent.get_context(node.parents, node, self._dst_model)

        # translate the source code; if it's too long use the chunk file agent
        # to translate it in parts
        if len(source_code) > 4096:
            chunks = self._chunk_file_agent.process_file(source_code)
            translation = ""
            for chunk in chunks:
                translation += self._get_translation(context, chunk, node)
        else:
            translation = self._get_translation(context, source_code, node)

        # write the translation to the output repo
        self._write_file(node.rel_path, translation)


    def _get_translation(self, context: str, source_code: str, file: FileNode) -> str:
        """ Get the translation for a region of code using the provided context. """
        prompt_config_src = self._input_repo.get_meta_dict()
        prompt_config_dst = self._dst_config
        filename = file.rel_path
        filename_desc = prompt_config_dst["filename_desc"]
        exts_str = ", ".join(v for v in ac.type_to_ext[filename_desc.lower()].values())

        prompt = ac.PROMPT_TEMPLATE.format(
            src_model=self._src_model,
            filename=filename,
            dst_model=self._dst_model,
            source_code=source_code,
            context=context,
            exts=exts_str,
            filename_desc=filename_desc
        )

        if filename == prompt_config_src["main_filename"]:
            prompt += ("\n" + ac.MAIN_ADDENDUM.format(
                ex_run_cmd=prompt_config_dst["ex_run_cmd"],
                ex_run_desc=prompt_config_dst["ex_run_desc"]))

        if filename == prompt_config_src["build_filename"]:
            prompt += ("\n" + ac.MAKEFILE_ADDENDUM.format(
                dst_model=self._dst_model,
                exts=exts_str,
                filename_desc=filename_desc,
                ex_build_cmd=prompt_config_dst["ex_build_cmd"],
                ex_build_desc=prompt_config_dst["ex_build_desc"]))

        print("Requesting file translation...")

        return self.generate(prompt)
