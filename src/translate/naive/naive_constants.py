SYSTEM_TEMPLATE: str = """You are a helpful coding assistant.
You are helping a software developer translate a codebase from the {src_model} execution model to the {dst_model} execution model.
Writing correct, fast code is important, so take some time to think before responding to any query."""

PROMPT_TEMPLATE: str = """Below is a codebase written in the {src_model} execution model. We are translating it to the {dst_model} execution model.
Here is the file tree of the entire repository:

{file_tree}

Here is the code for each file in the codebase:

{all_files}

Translate the {filename} file to the {dst_model} execution model. Output the translated file in one code block. Assume {exts} filenames whenever referring to other files as this will be a {filename_desc} code.
"""

MAIN_ADDENDUM: str = """This file includes the main function. Please ensure the command line interface after translation still works as expected, so that, for example, `{ex_run_cmd}` still works to run the code with {ex_run_desc}.
"""

BUILD_ADDENDUM: str = """This file is a {build_filename}. Please output a {new_build_filename} converted to compile this code as a {dst_model} code. Assume {exts} filenames as this will be a {filename_desc} code, and that the user will compile this code using, for example, `{ex_build_cmd}` to build the code for {ex_build_desc}.
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
