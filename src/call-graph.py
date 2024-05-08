#!/usr/bin/env python3

# std imports
import sys
import networkx as nx
import pydot
import argparse
import os
import subprocess
from typing import Optional, List

# local imports
from repo import Repo

class CallGraph:
    _graph: nx.DiGraph
    _repo: Repo

    def __init__(self, repo: Repo):
        """construct a call graph from a repository"""
        self._repo = repo
        code_files = [f for f in self._repo.get_all_filenames(relpaths=True)
                      if self._repo.is_code_file(f)]
        for f in code_files:
            subprocess.check_output(['clang++', '-emit-llvm', '-S',
                                     '--cuda-gpu-arch=sm_80', '-Iinclude',
                                     '-std=c++17', '-g',
                                     '-L/opt/nvidia/hpc_sdk/Linux_x86_64/23.1/cuda/12.0/lib64',
                                     '-L/global/common/software/nersc/pe/gpu/llvm/17.0.6/lib',
                                     '-lcudart_static', '-ldl', '-lrt',
                                     '-pthread', f, '-o', f + '.ll'])
            subprocess.check_output(['opt', '-p dot-callgraph', f,
                                     '-disable-output', '-o',
                                     f + '.mangled.dot'])
            subprocess.check_output(['cat', f + '.mangled.dot', '|', 'c++filt',
                                     '|', 'sed',
                                     r"'s,>,\\>,g; s,-\\>,->,g; s,<,\\<,g'",
                                     '|', 'gawk',
                                     r"'/external node/{id=$1} $1 != id'", '>',
                                     f + '.dot'])
            dot_files.append(f + '.dot')

        graphs = []
        for f in dot_files:
            with open(f) as f:
                graphs.append(nx.drawing.nx_pydot.read_dot(f))

        for graph in graphs:
            mapping = {node: '"' + nx.get_node_attributes(graph, 'label')[node][2:-2] +
                       '"' for node in graph.nodes()}
            nx.relabel_nodes(graph, mapping, copy=False)

        self._graph = nx.compose_all(graphs)

    def add_edge(self, src: str, dst: str):
        """add an edge to the call graph"""
        self._graph.add_edge(src, dst)

    def add_edges(self, edges: List[tuple]):
        """add multiple edges to the call graph"""
        self._graph.add_edges_from(edges)

    def get_call_graph(self) -> nx.DiGraph:
        """get the call graph"""
        return self._graph

    def get_call_graph_dot(self) -> str:
        """get the call graph in dot format"""
        return nx.drawing.nx_pydot.to_pydot(self._graph).to_string()

    def save_call_graph_dot(self, path: str):
        """save the call graph in dot format"""
        nx.drawing.nx_pydot.to_pydot(self._graph).write_dot(path)

    def save_call_graph_png(self, path: str):
        """save the call graph in png format"""
        nx.drawing.nx_pydot.to_pydot(self._graph).write_png(path)
