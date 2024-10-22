#!/usr/env/bin python3
'''
Run all generated code repositories
author: Josh Davis
date: October 2024
'''

from argparse import ArgumentParser
import json
import os

from tqdm import tqdm

def get_args():
    parser = ArgumentParser(description="Compile and run all the generated code repositories.")
    parser.add_argument("translations_root", type=str, help="Root directory of the generated code repositories.")
    parser.add_argument("-o", "--output", type=str, help="Output JSON file containing the results.")
    parser.add_argument("--scratch-dir", type=str, help="If provided, put scratch files here.")
    parser.add_argument("--apps", nargs="+", type=str, help="List of applications to run.")
    parser.add_argument("--models", nargs="+", type=str, help="List of execution models to run.", choices=["omp"])
    parser.add_argument("--dry", action="store_true", help="Dry run. Do not actually compile or run the code repositories.")
    parser.add_argument("--overwrite", action="store_true", help="If outputs are already in DB for a given prompt, then overwrite them. Default behavior is to skip existing results.")
    parser.add_argument("--hide-progress", action="store_true", help="If provided, do not show progress bar.")
    parser.add_argument("--build-only", action="store_true", help="If provided, only build the code repositories, do not run.")
    parser.add_argument("--run-only", action="store_true", help="If provided, only run the code repositories, do not build.")
    parser.add_argument("--early-exit-runs", action="store_true", help="If provided, stop evaluating a model output after the first run configuration fails.")
    parser.add_argument("--build-config", type=str, default="build-config.json", help="Config for how to build samples.")
    parser.add_argument("--run-config", type=str, default="run-config.json", help="Config for how to run samples.")
    parser.add_argument("--build-timeout", type=int, default=30, help="Timeout in seconds for building a program.")
    parser.add_argument("--run-timeout", type=int, default=120, help="Timeout in seconds for running a program.")
    parser.add_argument("--log-build-output", action="store_true", help="On all builds, display the stdout of the build process.")
    parser.add_argument("--log-build-errors", action="store_true", help="On build error, display the stderr of the build process.")
    parser.add_argument("--log-run-output", action="store_true", help="On all runs, display the stdout of the run process.")
    parser.add_argument("--log-run-errors", action="store_true", help="On run error, display the stderr of the run process.")
    return parser.parse_args()
