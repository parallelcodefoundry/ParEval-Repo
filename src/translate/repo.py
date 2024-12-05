""" Helper class for getting data from a repository directory.
    Supports utilities such as getting a file tree, file contents, etc.

    author: Daniel Nichols
    date: April 2024
"""
# std imports
import os
from typing import Optional, List
import json


class Repo:

    _path: os.PathLike
    _file_tree: dict
    _exp_meta: dict

    def __init__(self, path: os.PathLike, exp_meta: dict):
        self._path = os.path.abspath(path)
        self._file_tree = self._get_file_tree_dict(path)
        self._file_tree = self._file_tree['contents']
        if len(self._file_tree) == 1:
            self._file_tree = self._file_tree[0]['contents']
        self._exp_meta = exp_meta

    @classmethod
    def from_json(self, exp_meta: os.PathLike):
        with open(exp_meta, 'r') as f:
            self._exp_meta = json.load(f)
        implicit_path = os.path.abspath(os.path.join(os.path.dirname(exp_meta), "repo"))
        explicit_path_steps = self._exp_meta['path'].split(os.path.sep)
        if self._exp_meta['path'] != os.path.sep.join(implicit_path.split(os.path.sep)[-len(explicit_path_steps):]):
            raise ValueError("The provided path in the exp_meta file does not match the path of the exp_meta file.")
        return self(implicit_path, self._exp_meta)

    def get_meta_dict(self) -> dict:
        return self._exp_meta

    def get_file_tree_dict(self) -> dict:
        return self._file_tree

    def get_file_tree_str(self, ascii: bool = True, max_depth: Optional[int] = None) -> str:
        return "\n".join(self._get_file_tree_str(self._file_tree, ascii=ascii, max_depth=max_depth)) + "\n"

    def get_all_filenames(self, relpaths: bool = False) -> List[os.PathLike]:
        """ get all the files names in this._path as relative paths """
        all_files = [os.path.join(dp, f) for dp, dn, filenames in os.walk(self._path) for f in filenames]
        if relpaths:
            return [os.path.relpath(f, self._path) for f in all_files]
        return all_files

    def get_file_contents(self, rel_path: Optional[os.PathLike] = None, full_path: Optional[os.PathLike] = None) -> str:
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

        # read the file contents
        with open(full_path, 'r', encoding='ascii', errors='replace') as f:
            return f.read()

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
