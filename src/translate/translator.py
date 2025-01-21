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
    _dst_config: dict
    _log_interactions: bool
    _dry: bool # todo: change this to be a naive arg only
    _hide_progress: bool

    def __init__(
            self,
            input_repo: Repo,
            output_repo: os.PathLike,
            src_model: str,
            dst_model: str,
            dst_config: dict,
            log_interactions: bool = False,
            dry: bool = False,
            hide_progress: bool = False
    ):
        self._input_repo = input_repo
        self._output_fpath = output_repo
        self._src_model = src_model
        self._dst_model = dst_model
        self._dst_config = dst_config
        self._log_interactions = log_interactions
        self._dry = dry
        self._hide_progress = hide_progress

    @abstractmethod
    def translate(self):
        pass
