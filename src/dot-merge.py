#!/usr/bin/env python3
import sys
import networkx as nx
import pydot
import argparse

# Parse arguments from command line
parser = argparse.ArgumentParser(description='Merge multiple dot files into '\
                                 'one')
parser.add_argument('dotfiles', metavar='dot_file', type=str, nargs='+',
                    help='dot files to merge, separated by space')
parser.add_argument('outputname', metavar='output_name', type=str,
                    help='output file name template, any extensions will be '\
                    'appended')
parser.add_argument('--save-png', action='store_true',
                    help='save the merged graph as a png file')
parser.add_argument('--verbose', action='store_true',
                    help='verbose output')
args = parser.parse_args()
files = args.dotfiles
output = args.outputname

# Read each file and create networkx graph from it
graphs = []
for file in files:
    with open(file) as f:
        graph = nx.drawing.nx_pydot.read_dot(f)
        graphs.append(graph)

# For each graph, relabel nodes with the label (rather than node ID from opt)
# Also remove the curly braces around the labels
for graph in graphs:
    mapping = {node: '"' + nx.get_node_attributes(graph, 'label')[node][2:-2] +
               '"' for node in graph.nodes()}
    nx.relabel_nodes(graph, mapping, copy=False)

# Merge all graphs into one and convert to dot
graph = nx.compose_all(graphs)
graph = nx.drawing.nx_pydot.to_pydot(graph)

# Output
if args.verbose:
    print(graph.to_string())

if args.save_png:
    graph.write_png(output + '.png')

graph.write_dot(output + '.dot')
