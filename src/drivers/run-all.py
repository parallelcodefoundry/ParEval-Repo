#!/usr/bin/env python3
'''
Run all generated code repositories
author: Josh Davis
date: October 2024
'''
# std imports
from argparse import ArgumentParser
import json
import os
import logging
import shutil
import tempfile
import tarfile
import sys
from typing import Dict, List

# tpl imports
from alive_progress import alive_bar
import pandas as pd

# local imports
from util import await_input, setup_tempdir, dict_merge, update_results
from build import build_repo
from run import run_repo

def get_args() -> ArgumentParser:
    ''' Get command line arguments.
    '''
    parser = ArgumentParser(description="Compile and run all the generated code repositories.")
    parser.add_argument("translations_root", type=str,
                        help="Root directory of the generated code repositories.")
    parser.add_argument("-o", "--output", type=str, default="results.json",
                        help="Output JSON file containing the results.")
    parser.add_argument("--scratch-dir", type=str, default="scratch",
                        help="If provided, put scratch files here.")
    parser.add_argument("--save-temps", type=str,
                        help="If provided, save temporary files to the provided directory.")
    parser.add_argument("-a", "--apps", nargs="+", type=str,
                        help="List of applications to run, case-insensitive.")
    parser.add_argument("-m", "--models", nargs="+", type=str,
                        help="List of dest execution models to run, case-insensitive.",
                        choices=["openmp-offload", "kokkos"])
    parser.add_argument("-p", "--prompt-strategies", nargs="+", type=str,
                        help="List of prompt strategies to run, case-insensitive.")
    parser.add_argument("-y", "--yes-to-all", action="store_true",
                        help="If provided, automatically answer yes to all prompts.")
    parser.add_argument("-d", "--dry", action="store_true",
                        help="Dry run. Do not actually compile or run the code repositories.")
    parser.add_argument("-f", "--force-overwrite", action="store_true",
                        help="If outputs are already in DB for a given prompt, overwrite them.")
    parser.add_argument("--hide-progress", action="store_true",
                        help="If provided, do not show progress bar.")
    parser.add_argument("-t", "--target-path", type=str, default="targets",
                        help="Path to target repos with ground truths and config files.")
    parser.add_argument("--system-config", type=str, default="config/perlmutter-config.json",
                        help="Config for system-specific options used to configure builds.")
    parser.add_argument("--log-build-output", action="store_true",
                        help="On all builds, display the stdout of the build process.")
    parser.add_argument("--log-build-errors", action="store_true",
                        help="On build error, display the stderr of the build process.")
    parser.add_argument("--log-run-output", action="store_true",
                        help="On all runs, display the stdout of the run process.")
    parser.add_argument("--log-run-errors", action="store_true",
                        help="On run error, display the stderr of the run process.")
    parser.add_argument("--log", choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"],
                        default="INFO", type=str.upper, help="Logging level.")
    return parser.parse_args()

def extract_tarballs(translations_root: str, remove_tarball: bool = False):
    """
    Find all tarballs in translations_root and extract them.
    Optionally remove the tarball if remove_tarball is True.
    """
    # Note: Need to check if corresponding output-repo already exists (need to delete it before extracting the tarball again)
    # Idea: Could have a list of extracted repos and after running the drivers, remove those directories
    for root, _, files in os.walk(translations_root):
        for filename in files:
            if filename.endswith(".tar.gz"):
                tarball_path = os.path.join(root, filename)
                print(f"Found tarball: {tarball_path} and extracting it.")
                with tarfile.open(tarball_path, "r:gz") as tar:
                    tar.extractall(path=root)
                if remove_tarball:
                    os.remove(tarball_path)
                    print(f"Removed {tarball_path} after extraction.")

def parse_metadata(args: ArgumentParser, exp_meta: Dict[str, str],
                   metadata_found: Dict[str, List]) -> Dict[str, str]:
    ''' Parse the metadata from the experiment_metadata.json file.
    '''
    app = exp_meta["app"]
    if args.apps and app.lower() not in [s.lower() for s in args.apps]:
        logging.debug(f"Skipping {app} because it is not in {args.apps}.")
        return None
    if app not in metadata_found["apps_found"]:
        metadata_found["apps_found"].append(app)

    dest_model = exp_meta["dest_model"]
    if dest_model == "omp":
        dest_model = "openmp-offload"
        exp_meta["dest_model"] = "openmp-offload"
    if args.models and dest_model.lower() not in [s.lower() for s in args.models]:
        logging.debug(f"Skipping {dest_model} because it is not in {args.models}.")
        return None
    if dest_model not in metadata_found["dest_models_found"]:
        metadata_found["dest_models_found"].append(dest_model)

    prompt_strategy = exp_meta["prompt_strategy"]
    if isinstance(prompt_strategy, list):
        prompt_strategy = "-".join(prompt_strategy)
    if args.prompt_strategies and prompt_strategy.lower() not in [s.lower() for s in args.prompt_strategies]:
        logging.debug(f"Skipping {prompt_strategy} because it is not in {args.prompt_strategies}.")
        return None
    if prompt_strategy not in metadata_found["prompt_strategies_found"]:
        metadata_found["prompt_strategies_found"].append(prompt_strategy)

    llm_name = exp_meta["llm_name"]
    if llm_name not in metadata_found["llms_found"]:
        metadata_found["llms_found"].append(llm_name)

    source_model = exp_meta["source_model"]
    if source_model not in metadata_found["source_models_found"]:
        metadata_found["source_models_found"].append(source_model)

    return exp_meta


def log_metadata_found(metadata_found: Dict[str, List], code_repos: List[Dict[str, str]]):
    ''' Log the metadata found.
    '''
    logging.info(f"Found {len(code_repos)} code repositories.")
    logging.debug(f"Found apps: {metadata_found['apps_found']}")
    logging.debug(f"Found prompt strategies: {metadata_found['prompt_strategies_found']}")
    logging.debug(f"Found llms: {metadata_found['llms_found']}")
    logging.debug(f"Found dest models: {metadata_found['dest_models_found']}")
    logging.debug(f"Found source models: {metadata_found['source_models_found']}")
    logging.debug("Code repositories:")
    logging.debug("\n" + json.dumps(code_repos, indent=4))


def gather_code_repos(args: ArgumentParser, results: Dict[str, List]) -> List[Dict[str, str]]:
    ''' Gather all the generated code repositories. Root directory format is:
        translations_root/app/case-name/output-number/, including
        experiment_metadata.json and repo/ under each output-number directory.
        repo/ contains the generated code. Want to create list of dictionaries
        of the form:
        {
            "app": app,
            "prompt_strategy": prompt_strategy,
            "llm_name": llm_name,
            "source_model": source_model,
            "dest_model": dest_model,
            "output_number": output_number,
            "path": path
        }
        where all entries are read in from the experiment_metadata.json file.
    '''
    code_repos = []

    # For logging what we find
    metadata_found = {
        "apps_found" : [],
        "dest_models_found" : [],
        "source_models_found" : [],
        "llms_found" : [],
        "prompt_strategies_found" : [],
    }

    for dirpath, _, filenames in os.walk(args.translations_root):
        for filename in filenames:
            if filename == "experiment_metadata.json":
                exp_meta_path = os.path.join(dirpath, "experiment_metadata.json")
                repo_path = os.path.join(dirpath, "repo")
                if not os.path.isfile(exp_meta_path):
                    logging.info(f"Could not find experiment_metadata.json under {dirpath}, skipping.")
                    continue
                if not os.path.isdir(repo_path):
                    logging.error(f"Could not find repo under {dirpath}.")
                    raise FileNotFoundError(f"Could not find repo under {dirpath}.")

                with open(exp_meta_path, "r", encoding="utf-8") as f:
                    exp_meta = json.load(f)

                    # Clean up the path in the experiment metadata
                    exp_meta["path"] = repo_path

                    # Parse the metadata
                    exp_meta = parse_metadata(args, exp_meta, metadata_found)
                    if exp_meta:
                        code_repos.append(exp_meta)
                    else:
                        continue

                    # Check if there is an entry in the results dict matching the current repo path
                    output_path = exp_meta["path"]
                    if output_path in results["path"]:
                        logging.warning(f"Skipping duplicate code repository: {output_path}")
                    else:
                        dict_merge(results, exp_meta)
                        logging.debug(f"Found code repository: {output_path}")

    # Log the metadata found
    log_metadata_found(metadata_found, code_repos)

    return code_repos


def safe_get_cols() -> int:
    ''' Get the number of columns in the terminal.
    '''
    try:
        max_cols = os.get_terminal_size().columns
    except OSError as error:
        if error.errno == 25:
            max_cols = 80
        else:
            raise error
    return max_cols


def startup_from_args(args: ArgumentParser) -> os.PathLike:
    ''' Startup from command line arguments.
    '''
    # Set up logging
    numeric_level = getattr(logging, args.log, None)
    if not isinstance(numeric_level, int):
        raise ValueError(f"Invalid log level: {args.log}")
    logging.basicConfig(format="%(asctime)s [%(levelname)s] -- %(message)s", level=numeric_level)

    # Make sure the translations root directory exists
    if not os.path.exists(args.translations_root):
        raise FileNotFoundError(f"Translations root directory {args.translations_root} does not exist.")

    # Warn user if using dry
    if args.dry:
        logging.warning("Running in dry mode. No code will be compiled or run!")

    # Confirm user knows the script runs LLM code
    logging.warning("This script will compile and run code generated by an LLM. " +
                    "It is recommended that you run this script in a sandboxed environment.")
    if not args.yes_to_all:
        response = await_input("Continue knowing that this script runs LLM-generated code? [y/n]: ", lambda r: r.lower() in ["y", "n", "yes", "no"])
        if response.lower() not in ["y", "yes"]:
            logging.warning("Exiting.")
            sys.exit(0)

    # Check that force overwrite is set if output file already exists
    if os.path.exists(args.output) and not args.force_overwrite:
        raise FileExistsError(f"Output file {args.output} already exists. Use --force-overwrite to overwrite.")

    # Check that the scratch directory exists
    scratch = os.path.abspath(args.scratch_dir)
    if not os.path.exists(scratch):
        logging.info(f"Creating scratch directory: {scratch}")
        os.makedirs(scratch)

    return scratch


def main():
    ''' Main function.
    '''
    args = get_args()

    # Startup from command line arguments
    scratch = startup_from_args(args)

    # Extract tarballs
    extract_tarballs(args.translations_root, remove_tar=False)

    # Load system config
    with open(args.system_config, "r", encoding="utf-8") as f:
        system_config = json.load(f)
    logging.debug(f"Loaded system config: {system_config}")

    # Create dict of empty lists to store results with columns for each metadata field
    results = {
        "app": [],
        "prompt_strategy": [],
        "llm_name": [],
        "source_model": [],
        "dest_model": [],
        "output_number": [],
        "path": [],
        "build_result_debug": [],
        "build_stdout_debug": [],
        "build_stderr_debug": [],
        "run_results_debug": [],
        "run_exec_checks_debug": [],
        "run_stdouts_debug": [],
        "run_stderrs_debug": [],
        "tempdir_path": []
    }

    # Gather all the code repositories
    code_repos = gather_code_repos(args, results)

    # Build and run each code repository
    max_cols = safe_get_cols()
    with alive_bar(len(code_repos)*2, title="Building and running code repositories", max_cols=max_cols, disable=args.hide_progress) as pbar:
        for code_repo in code_repos:
            with tempfile.TemporaryDirectory(dir=scratch) as tempdir:
                logging.debug(f"Temporary directory created: {tempdir}")
                setup_tempdir(tempdir, code_repo)

                logging.debug(f"Building code repository: {code_repo['path']}")
                results_row = build_repo(code_repo, system_config, args, tempdir)
                update_results(results, results_row)
                pbar()

                logging.debug(f"Running code repository: {code_repo['path']}")
                results_row = run_repo(code_repo, system_config, args, tempdir)

                # Copy temporary directory to save_temps if provided
                if args.save_temps:
                    tempdir_name = os.path.basename(tempdir)
                    tempdir_path = os.path.join(args.save_temps, tempdir_name)
                    logging.info(f"Saving temporary directory to {tempdir_path}.")
                    shutil.copytree(tempdir, tempdir_path)
                    results_row["tempdir_path"] = str(tempdir_path)

                update_results(results, results_row)
                pbar()

    # Convert results dict to dataframe
    results_df = pd.DataFrame.from_dict(results)

    # Write the results dataframe to the output filename
    logging.info(f"Writing results to {args.output}.")
    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(json.loads(results_df.to_json(orient="index")), f, indent=4)


if __name__ == "__main__":
    main()
