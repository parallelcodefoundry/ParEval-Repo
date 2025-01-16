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
from typing import List, Dict, Literal

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from agent.dependency_agent import DependencyAgent, FileNode
from agent.chunk_agent import ChunkFileAgent
from agent.context_agent import ContextAgent
from generator_mixin import GeneratorMixin
from repo import Repo


class TopDownAgentTranslator(Translator, GeneratorMixin):

    _dependency_agent: DependencyAgent
    _chunk_file_agent: ChunkFileAgent
    _context_agent: ContextAgent

    def __init__(
        self,
        input_repo: Repo,
        output_repo: os.PathLike,
        src_model: str,
        dst_model: str,
        llm_name: str,
        backend: Literal["openai", "gemini", "hf", "local"] = "openai",
    ):
        super().__init__(input_repo, output_repo, src_model, dst_model)
        GeneratorMixin.__init__(self, backend, llm_name)

        self._dependency_agent = DependencyAgent(generator=self)
        self._chunk_file_agent = ChunkFileAgent(generator=self)
        self._context_agent = ContextAgent(generator=self)

    @staticmethod
    def add_args(parser: 'ArgumentParser'): # type: ignore # noqa: F821
        """ Add arguments for the top-down agent translation method.
        """
        parser.add_argument("--agent-backend", choices=["openai", "gemini", "hf", "local"], default="openai", help="The backend to use for translation.")
        parser.add_argument("--agent-llm-name", type=str, help="The name of the LLM to use for translation.")

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
        with open(input_file_path, 'r') as f:
            return f.read()


    def _write_file(self, rel_path: str, contents: str):
        """ Write the contents to a file in the output repository.
        """
        print(f"Writing file {self._output_fpath}/{rel_path}...")
        output_file_path = os.path.join(self._output_fpath, rel_path)
        with open(output_file_path, 'w') as f:
            f.write(contents)


    # override
    def translate(self, dry: bool = False, log_interactions: bool = False):
        """ Use the top-down method to translate the entire repository.
        """
        print("Constructing dependency graph...")
        dep_tree = self._dependency_agent.construct_dependency_graph(self._input_repo.path)

        # walk down the tree and translate each file
        for node in dep_tree:
            self._translate_node(node)


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
                translation += self._get_translation(context, chunk)
        else:
            translation = self._get_translation(context, source_code)


        # write the translation to the output repo
        self._write_file(node.rel_path, translation)


    def _get_translation(self, context: str, source_code: str) -> str:
        """ Get the translation for a region of code using the provided context. """
        prompt = f"""Your task is to translate the following code snippet into {self._dst_model}:
                    ```
                    {source_code}
                    ```
                    The following is context for the translation task:
                    ```
                    {context}
                    ```
                    """
        print("Requesting file translation...")

        return self.generate(prompt)
