from typing import List
from generator_mixin import GeneratorMixin
from dependency_agent import FileNode

class ContextAgent:
    def __init__(self, generator: GeneratorMixin):
        self._generator = generator

    def get_context(self, parent_nodes: List[FileNode]) -> str:
        translated_codes = []
        for parent in parent_nodes:
            translated_code = self._read_translated_file(parent.rel_path)
            if translated_code:
                translated_codes.append(f"File: {parent.rel_path}\n{translated_code}")

        if not translated_codes:
            return ""

        combined_translations = "\n\n".join(translated_codes)
        prompt = f"""Given the following translated code snippets from parent files:

{combined_translations}

Extract only the relevant context that is necessary for translating dependent files."""
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