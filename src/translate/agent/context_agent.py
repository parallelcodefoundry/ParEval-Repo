from typing import List
from generator_mixin import GeneratorMixin
from agent.dependency_agent import FileNode

class ContextAgent:
    def __init__(self, generator: GeneratorMixin):
        self._generator = generator

    def get_context(self, parent_nodes: List[FileNode], node: FileNode, dst_model: str) -> str:
        print("Extracting context for dependent files...")
        translated_codes = []
        for parent in parent_nodes:
            translated_code = self._read_translated_file(parent.rel_path)
            if translated_code:
                translated_codes.append(f"File: {parent.rel_path}\n{translated_code}")

        if not translated_codes:
            return ""

        combined_translations = "\n\n".join(translated_codes)
        prompt = f"""You are assisting with the translation of an application to {dst_model}. The next file in the application to translate is {node.rel_path}. Below are the files already translated:

{combined_translations}

Please extract any code snippets from the above translated files that may be relevant to translating {node.rel_path}. Do not provide any new code or translations, just rewrite the relevant context in the form of code snippets with some explanation."""

        context = self._generator.generate(prompt)
        return context.strip() if context else ""

    def _read_translated_file(self, rel_path: str) -> str:
        output_repo = self._generator.output_repo
        output_file_path = output_repo.path / rel_path
        if output_file_path.exists():
            with open(output_file_path, 'r') as f:
                return f.read()
        else:
            return ""
