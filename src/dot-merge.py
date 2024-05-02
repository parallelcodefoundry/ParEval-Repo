#!/usr/bin/env python3
import sys
import networkx as nx
import pydot
import argparse

parser = argparse.ArgumentParser(description='Merge multiple dot files into one')
parser.add_argument('files', metavar='file', type=str, nargs='+',
                    help='dot files to merge, separated by space')
parser.add_argument('output', metavar='output', type=str,
                    help='output file name, png extension will be appended')
parser.add_argument('--verbose', action='store_true',
                    help='verbose output')
args = parser.parse_args()
files = args.files
output = args.output

# Read each file and create networkx graph from it
graphs = []
for file in files:
    with open(file) as f:
        graph = nx.drawing.nx_pydot.read_dot(f)
        graphs.append(graph)

# For each graph, relabel nodes with the label
# We remove the curly braces around the labels
for graph in graphs:
    mapping = {node: '"' + nx.get_node_attributes(graph, 'label')[node][2:-2] + '"' for node in graph.nodes()}
    nx.relabel_nodes(graph, mapping, copy=False)

graph = nx.compose_all(graphs)
graph = nx.drawing.nx_pydot.to_pydot(graph)

if args.verbose:
    print(graph.to_string())

graph.write_png(output + '.png')
