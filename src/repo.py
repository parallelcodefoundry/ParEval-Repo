""" Helper class for getting data from a repository directory.
    Supports utilities such as getting a file tree, file contents, etc.,
    as well as getting compilation entries for files in the repository.

    author: Daniel Nichols, Josh Davis
    date: April 2024
"""
# std imports
import os
import json
from typing import Optional, List


class Repo:

    _path: os.PathLike
    _file_tree: dict
    _compile_database: CompilationDatabase

    def __init__(self, path: os.PathLike, compile_database_json: str):
        self._path = os.path.abspath(path)
        self._file_tree = self._get_file_tree_dict(path)
        self._file_tree = self._file_tree['contents']
        if len(self._file_tree) == 1:
            self._file_tree = self._file_tree[0]['contents']
        self._compile_database = CompilationDatabase.from_json(compile_database_json)

    def get_file_tree_dict(self) -> str:
        return self._file_tree

    def get_file_tree_str(self, ascii: bool = True, max_depth: Optional[int] = None) -> str:
        return "\n".join(self._get_file_tree_str(self._file_tree, ascii=ascii, max_depth=max_depth)) + "\n"

    def get_all_filenames(self, relpaths: bool = False) -> List[os.PathLike]:
        """ get all the files names in this._path as relative paths """
        all_files = [os.path.join(dp, f) for dp, dn, filenames in os.walk(self._path) for f in filenames]
        if relpaths:
            return [os.path.relpath(f, self._path) for f in all_files]
        return all_files

    def get_all_code_filenames(self, relpaths: bool = False) -> List[os.PathLike]:
        """ get all the CUDA C/C++ code file names in this._path as relative paths """
        all_files = self.get_all_filenames(relpaths=False)
        code_files = [f for f in all_files if self.is_code_file(f)]
        if relpaths:
            return [os.path.relpath(f, self._path) for f in code_files]
        return code_files

    def is_code_file(self, filename: os.PathLike) -> bool:
        """ check if a file is a CUDA C/C++ code file """
        endings = ['.cu', '.cuh', '.cpp', '.h', '.hpp', '.c', '.cc', '.hh', '.cxx', '.hxx', '.C', '.H']
        return any([filename.endswith(e) for e in endings])

    def get_full_path(self, rel_path: Optional[os.PathLike] = None, full_path: Optional[os.PathLike] = None) -> os.PathLike:
        # make sure only one and at least one is provided
        if (rel_path is None and full_path is None) or (rel_path is not None and full_path is not None):
            raise ValueError("Either a relative or full path must be provided.")

        # check that full path is in the repo
        if full_path is not None:
            if os.path.commonpath([self._path, full_path]) != self._path:
                raise ValueError("The provided path is not in the repository.")

        # get the full path
        full_path = full_path or os.path.join(self._path, rel_path)

        # check that the file exists
        if not os.path.exists(full_path):
            raise FileNotFoundError("The provided path does not exist.")

        return full_path

    def get_file_contents(self, rel_path: Optional[os.PathLike] = None, full_path: Optional[os.PathLike] = None) -> str:
        full_path = self.get_full_path(rel_path=rel_path, full_path=full_path)

        # read the file contents
        with open(full_path, 'r') as f:
            return f.read()

    def get_compilation_entry(self, rel_path: Optional[os.PathLike] = None, full_path: Optional[os.PathLike] = None) -> CompilationEntry:
        """ get the compilation entry for a file """
        full_path = self.get_full_path(rel_path=rel_path, full_path=full_path)

        # get the compilation entry for the file
        return self._compile_database.get_compilation_entry(full_path)

    def _get_file_tree_dict(self, tree_root: os.PathLike) -> dict:
        # build a tree of all files in the repo
        tree = {"name": os.path.basename(tree_root)}

        if os.path.isdir(tree_root):
            # If the given path is a directory, recursively populate the tree
            tree["type"] = "directory"
            tree["contents"] = [self._get_file_tree_dict(os.path.join(tree_root, item)) for item in os.listdir(tree_root)]
        else:
            # If the given path is a file, record it in the tree
            tree["type"] = "file"

        return tree

    def _get_file_tree_str(self, root: List[dict], prefix: str = "", ascii: bool = False, depth: int = 0, max_depth: Optional[int] = None):
        """ Get the string representation of a file tree.
            It will be formatted as:

            root/
            ├── subdir/
            |   ├── file1
            |   └── file2
            └── file3
        """
        space =  '    ' if ascii else '    '
        branch = '|   ' if ascii else '│   '
        tee =    '|-- ' if ascii else '├── '
        last =   '|__ ' if ascii else '└── '

        output = ""
        pointers = [tee] * (len(root) - 1) + [last]
        for pointer, child in zip(pointers, root):
            yield prefix + pointer + child['name'] + ("/" if child['type'] == 'directory' else "")
            if child['type'] == 'directory' and 'contents' in child and len(child['contents']) > 0 and (max_depth is None or depth < max_depth):
                extension = branch if pointer == tee else space
                yield from self._get_file_tree_str(child['contents'], prefix + extension, ascii=ascii, depth=depth+1, max_depth=max_depth)


class CompilationEntry:

    _directory: os.PathLike
    _arguments: List[str]
    _file: str
    _output: str

    def __init__(self, directory: os.PathLike, arguments: List[str], file: str, output: str):
        self._directory = directory
        self._arguments = arguments
        self._file = file
        self._output = output

    def get_llvm_ir_args(self) -> List[str]:
        """ return the arguments adjusted to compile to LLVM IR """
        args = self._arguments.copy()

        # if compiler is not clang, throw an error
        if 'clang' not in args[0]:
            raise ValueError("Only clang is supported for generating LLVM IR.")

        # remove the output file flag and whatever filename immediately follows it
        if '-o' in args:
            idx = args.index('-o')
            args.pop(idx)
            args.pop(idx)

        # remove any flags for steps other than assemble
        if '-c' in args:
            args.remove('-c')
        if '-E' in args:
            args.remove('-E')

        # remove all optimization flags (may not be necessary)
        #args = [a for a in args if not a.startswith('-O')]

        # disable inlining
        args.append('-fno-inline-functions')

        # add the flags to compile to LLVM IR
        args.append('-S')
        args.append('-emit-llvm')
        args.append('-o')
        args.append(self._file + '.ll')

        return args

    def get_directory(self) -> os.PathLike:
        return self._directory

    def get_arguments(self) -> List[str]:
        return self._arguments

    def get_file(self) -> str:
        return self._file

    def get_output(self) -> str:
        return self._output

    def to_dict(self) -> dict:
        return {"directory": self._directory, "arguments": self._arguments, "file": self._file, "output": self._output}

    @staticmethod
    def from_dict(data: dict):
        return CompilationEntry(data['directory'], data['arguments'], data['file'], data['output'])


class CompilationDatabase:

    _entries: List[CompilationEntry]

    def __init__(self, entries: List[CompilationEntry]):
        # filter down to just the first entry whose output ends in .o for each file
        filtered_entries = []
        seen_files = set()
        for entry in entries:
            output = entry.get_output()
            if output.endswith('.o') and output not in seen_files:
                seen_files.add(output)
                filtered_entries.append(entry)

        if len(filtered_entries) == 0:
            raise ValueError("No compilation entries found with output ending in .o")

        self._entries = filtered_entries

    def get_entries(self) -> List[CompilationEntry]:
        return self._entries

    def get_compilation_entry(self, filename: os.PathLike) -> CompilationEntry:
        for entry in self._entries:
            if os.path.samefile(entry.get_file(), filename):
                return entry
        raise ValueError(f"No compilation entry found for file {filename}")

    def to_dict(self) -> dict:
        return {"entries": [e.to_dict() for e in self._entries]}

    @staticmethod
    def from_dict(data: dict):
        return CompilationDatabase([CompilationEntry.from_dict(e) for e in data['entries']])

    @staticmethod
    def from_json(json_str: str):
        return CompilationDatabase.from_dict(json.loads(json_str))
