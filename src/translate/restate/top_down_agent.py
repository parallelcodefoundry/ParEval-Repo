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
from typing import Dict, Literal, Optional, List, Union
import re

# local imports
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from translator import Translator
from restate.dependency_agent import DependencyAgent, FileNode, FileType
from restate.chunk_agent import ChunkFileAgent
from restate.context_agent import ContextAgent
from restate import agent_constants as ac
from generator_mixin import GeneratorMixin
from repo import Repo

# tpl imports
from alive_progress import alive_it


class TopDownAgentTranslator(Translator, GeneratorMixin):
    """ A class to translate an entire repository using the top-down agent method.
    """

    _dependency_agent: DependencyAgent
    _chunk_file_agent: ChunkFileAgent
    _context_agent: ContextAgent
    _system_prompt: str
    _interactions_path: Optional[os.PathLike] = None
    _output_fpath: os.PathLike

    def __init__(
            self,
            input_repo: Repo,
            output_repos: List[os.PathLike],
            src_model: str,
            dst_model: str,
            dst_config: dict,
            llm_name: str,
            backend: Literal["openai", "gemini", "hf", "vllm", "local"] = "openai",
            log_interactions: bool = False,
            dry: bool = False,
            hide_progress: bool = False
    ):
        super().__init__(input_repo, output_repos, src_model, dst_model,
                         dst_config, log_interactions, dry, hide_progress)

        self._system_prompt = ac.SYSTEM_TEMPLATE.format(
            src_model=self._src_model,
            dst_model=self._dst_model)

        GeneratorMixin.__init__(self, backend, llm_name, system_prompt=self._system_prompt)

        # use the first output repo since agent doesn't support multiple outputs
        self._output_fpath = output_repos[0]

        if self._log_interactions:
            self._interactions_path = os.path.join(self._output_fpath, "interactions.txt")

            # create directories if necessary
            os.makedirs(os.path.dirname(self._interactions_path), exist_ok=True)

        self._dependency_agent = DependencyAgent(generator=self,
                                                 interactions_path=self._interactions_path)
        lang = "c" if self._dst_config["filename_desc"] == "C" else "cpp"
        self._chunk_file_agent = ChunkFileAgent(generator=self,
                                                interactions_path=self._interactions_path,
                                                language=lang)
        self._context_agent = ContextAgent(generator=self,
                                           interactions_path=self._interactions_path,
                                           output_fpath=self._output_fpath)

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

    def _read_file(self, rel_path: str) -> str:
        """ Read the contents of a file from the input repository.
        """
        input_file_path = os.path.join(self._input_repo.path, rel_path)
        with open(input_file_path, 'r', encoding="UTF-8") as f:
            return f.read()

    def _update_output_file_extension(self, fname: str) -> str:
        """ Return the filename with updated extension based on the destination model.
        """
        # Check if file has an extension
        if "." in fname:
            name, current_ext = os.path.splitext(fname)

            # Check if the extension is in the dict
            if current_ext in ac.ext_to_type:
                if self._dst_model == "cuda":
                    ext_category = self._dst_model
                else:
                    ext_category = self._input_repo.get_meta_dict()["filename_desc"]
                return name + ac.type_to_ext[ext_category.lower()][ac.ext_to_type[current_ext]]
        return fname


    def _postprocess(self, output: str) -> str:
        """ make sure there's only one codeblock and extract it
        """
        CODE_BLOCK_PATTERN = re.compile(r"```(?:[+\w]+)?\n(.*?)\n```", re.DOTALL)
        match = CODE_BLOCK_PATTERN.search(output)
        if match is None:
            raise ValueError(f"No code block found in output:\n{output}")
        return match.group(1)


    def _write_file(self, rel_path: str, contents: str):
        """ Write the contents to a file in the output repository.
        """
        output_file_path = os.path.join(self._output_fpath, "repo",
                                        self._update_output_file_extension(rel_path))

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
                "prompt_strategy": "restate",
                "llm_name": self._llm_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": int(str(repo_fpath).split("/")[-2][7:]),
                "path": repo_fpath,
                "inference_stats": self.get_stats()
            }
            json.dump(exp_meta_dict, f, indent=4)
        print(f"Wrote translation experiment metadata to {exp_meta_fpath}")


    def _log_interaction(self, prompt: str, response: str, reasoning: Union[str, None]):
        """ Log the prompt and raw LLM output to a text file.
        """
        with open(self._interactions_path, 'a', encoding="UTF-8") as f:
            f.write("PROMPT:\n")
            f.write(prompt + "\n")
            if reasoning is not None:
                f.write("REASONING:\n")
                f.write(reasoning + "\n")
            f.write("RESPONSE:\n")
            f.write(response + "\n\n")


    # override
    def translate(self):
        """ Use the top-down method to translate the entire repository.
        """
        print(f"Constructing dependency graph on {self._input_repo.path}...")
        dep_graph = self._dependency_agent.construct_dependency_graph(self._input_repo.path)

        # Create target version of dep_graph by renaming filenames in copy of dep_graph
        target_dep_graph = DependencyAgent.make_target_graph(dep_graph,
                                                             self._update_output_file_extension)

        # walk down the graph and translate each file
        max_cols = self._safe_get_columns()
        for node in alive_it(dep_graph,
                             title="Translating files",
                             max_cols=max_cols,
                             disable=self._hide_progress):
            if node.filetype == FileType.BUILD:
                self._translate_node(node, graph=target_dep_graph)
            else:
                self._translate_node(node)

        # dump experiment metadata
        self._write_metadata(os.path.join(self._output_fpath, "repo"))


    def _translate_node(self, node: FileNode, graph: Optional[List[FileNode]] = None):
        """ Translate a single file node using context from its dependencies.
        """
        print(f"Translating file {node.rel_path}...")

        # get the source code from the file
        source_code = self._read_file(node.rel_path)

        # get the context from the parent nodes
        context = self._context_agent.get_context(node.dependencies, node, self._dst_model)

        # translate the source code; if it's too long use the chunk file agent
        # to translate it in parts
        chunks = self._chunk_file_agent.chunk_file(source_code)
        if len(chunks) == 1:
            print("Requesting whole file translation...")
            response = self._get_translation(context, chunks[0], node, graph)
            translation = self._postprocess(response)
        else:
            translation = ""
            prev_chunk = ""
            for chunk in chunks:
                print(f"Requesting chunk translation... [{chunks.index(chunk) + 1}/{len(chunks)}]")
                response = self._get_translation(context, chunk, node, graph, prev_chunk=prev_chunk)
                chunk_translation = self._postprocess(response)
                translation += chunk_translation + "\n"
                prev_chunk = chunk_translation

        # write the translation to the output repo
        self._write_file(node.rel_path, translation)


    def _get_translation(self, context: str, source_code: str, file: FileNode, \
                         graph: Optional[List[FileNode]] = None, \
                         prev_chunk: Optional[str] = "") -> str:
        """ Get the translation for a region of code using the provided context.
        """
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

        if prev_chunk != "":
            prompt += ("\n" + ac.CHUNK_ADDENDUM.format(
                prev_chunk=prev_chunk,
                src_model=self._src_model,
                dst_model=self._dst_model))

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
                ex_build_desc=prompt_config_dst["ex_build_desc"],
                dep_graph=DependencyAgent.graph_to_str(graph)))

        response_obj = self.generate(prompt, temperature=0.2, top_p=0.95)[0]
        response, reasoning = response_obj.response, response_obj.reasoning

        if self._log_interactions:
            self._log_interaction(prompt, response, reasoning)

        return response
