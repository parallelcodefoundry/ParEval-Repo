import os
from typing import List, Optional
from generator_mixin import GeneratorMixin
from restate.dependency_agent import FileNode

class ContextAgent:

    _generator: GeneratorMixin
    _interactions_path: List[os.PathLike]
    _output_paths: List[os.PathLike]

    def __init__(self, generator: GeneratorMixin,
                 output_paths: List[os.PathLike],
                 interactions_paths: Optional[List[os.PathLike]] = None):
        self._generator = generator
        self._interactions_paths = interactions_paths
        self._output_paths = output_paths

    def get_contexts(self, dependencies: List[FileNode],
                     node: FileNode, dst_model: str) -> List[str]:
        """ Extract context for the translation task.
        """
        print("Extracting context for dependent files...")
        if not dependencies:
            return ""

        # For each output path, read over dependent files already translated
        # and extract relevant code snippets.
        prompts = []
        for output_path in self._output_paths:
            translated_codes = []
            for dependency in dependencies:
                translated_code = self._read_translated_file(dependency.rel_path,
                                                             output_path)
                if translated_code:
                    translated_codes.append(f"File: {dependency.rel_path}\n{translated_code}")

            combined_translations = "\n\n".join(translated_codes)
            prompt = f"""You are assisting with the translation of an application to {dst_model}. The next file in the application to translate is {node.rel_path}. Below are the files already translated:

{combined_translations}

Please extract any code snippets from the above translated files that may be relevant to translating {node.rel_path}. Do not provide any new code or translations, just rewrite the relevant context in the form of code snippets with some explanation."""

            prompts.append(prompt)

        response_obs = self._generator.generate_async(prompts, temperature=0.2, top_p=0.95)
        contexts = [r.response.strip() if r.response else "" for r in response_obs]
        return contexts

    def _read_translated_file(self, rel_path: os.PathLike, output_path: os.PathLike) -> str:
        """ Read the translated file from the output path.
        """
        output_file_path = os.path.join(output_path, rel_path)
        if os.path.exists(output_file_path):
            with open(output_file_path, 'r', encoding='utf-8') as f:
                return f.read()
        else:
            return ""
