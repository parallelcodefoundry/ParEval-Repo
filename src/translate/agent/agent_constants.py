SYSTEM_TEMPLATE: str = """You are a helpful coding assistant.
You are helping a software developer translate a codebase from the {src_model} execution model to the {dst_model} execution model."""

PROMPT_TEMPLATE: str = """Your task is to translate the following {src_model} code snippet from {filename} into {dst_model}:
```
{source_code}
```
The following is context for the translation task:
```
{context}
```
Output the translated snippet in one code block. Assume {exts} filenames whenever referring to other files as this will be a {filename_desc} code.
"""

MAIN_ADDENDUM: str = """This snippet is from the file that includes the main function. Please ensure the command line interface after translation still works as expected, so that, for example, `{ex_run_cmd}` still works to run the code with {ex_run_desc}.
"""

MAKEFILE_ADDENDUM: str = """This snippet is from a Makefile. Please output an equivalent Makefile snippet converted to compile this code as a {dst_model} code. Where relevant, assume {exts} filenames as this will be a {filename_desc} code, and that the user will compile this code using, for example, `{ex_build_cmd}` to build the code for {ex_build_desc}.
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
