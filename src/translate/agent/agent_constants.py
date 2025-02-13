SYSTEM_TEMPLATE: str = """You are a helpful coding assistant.
You are helping a software developer translate a codebase from the {src_model} execution model to the {dst_model} execution model."""

PROMPT_TEMPLATE: str = """Your task is to translate the following {src_model} code from {filename} into {dst_model}:
```
{source_code}
```
The following is context for the translation task, taken from other already-translated files in the application:
```
{context}
```
Output the translated code in one code block. Assume {exts} filenames whenever referring to other files as this will be a {filename_desc} code. To avoid repeating code that might translated in other snippets, please respect the organization of the existing code, and do not introduce functionality here if it should appear elsewhere in the codebase.
"""

CHUNK_ADDENDUM: str = """This snippet is from a chunk of a larger file. Assume that the rest of the file is also being translated from {src_model} to {dst_model}. For reference, here is the previous chunk of the file, already translated for you:
```
{prev_chunk}
```
Do not repeat the previous chunk in your output, but do use it as context for your translation. Do not include any header files that would already be included earlier in the file.
"""

MAIN_ADDENDUM: str = """You are translating code from a file that includes the main function. As relevant, please ensure the command line interface after translation still works as expected, so that, for example, `{ex_run_cmd}` still works to run the code with {ex_run_desc}.
"""

MAKEFILE_ADDENDUM: str = """You are translating code taken from a Makefile. Please output an equivalent Makefile snippet converted to compile this code as a {dst_model} code. Where relevant, assume {exts} filenames as this will be a {filename_desc} code, and that the user will compile this code using, for example, `{ex_build_cmd}` to build the code for {ex_build_desc}. For reference, here are the file dependendencies of the full application:
```
{dep_graph}
```
"""

# Dicts of file extension mappings
ext_to_type: dict = {
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

type_to_ext: dict = {
    "cuda": {"code": ".cu", "header": ".cuh"},
    "c++": {"code": ".cpp", "header": ".hpp"},
    "c": {"code": ".c", "header": ".h"}
}
