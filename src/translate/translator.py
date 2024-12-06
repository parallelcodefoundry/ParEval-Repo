""" Abstract class with interface for translator implementations.
    It takes a path to a repository as an input and a path to an output
    repository as well as the source and destination execution model. The 
    translator will then translate the input repository to the output
    repository using the specified translation method.
"""
# std imports
from abc import ABC, abstractmethod
import os

# local imports
from repo import Repo


class Translator(ABC):

    _input_repo: Repo
    _output_fpath: os.PathLike
    _src_model: str
    _dst_model: str
    _output_id: int
    _app_name: str
    _llm_name: str
    _dst_config: os.PathLike

    def __init__(self, input_repo: Repo, output_repo: os.PathLike, src_model: str, dst_model: str, output_id: int, app_name: str, llm_name: str, dst_config: os.PathLike):
        self._input_repo = input_repo
        self._output_fpath = output_repo
        self._src_model = src_model
        self._dst_model = dst_model
        self._output_id = output_id
        self._app_name = app_name
        self._llm_name = llm_name
        self._dst_config = dst_config

    @abstractmethod
    def translate(self, dry: bool = False, log_interactions: bool = False):
        pass
