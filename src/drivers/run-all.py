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
import contextlib
import shutil

# tpl imports
from alive_progress import alive_bar
import pandas as pd
import tempfile

# local imports
from util import await_input, setup_tempdir, dict_merge, update_results
from build import build_repo
from run import run_repo

def get_args():
    parser = ArgumentParser(description="Compile and run all the generated code repositories.")
    parser.add_argument("translations_root", type=str, help="Root directory of the generated code repositories.")
    parser.add_argument("-o", "--output", type=str, help="Output JSON file containing the results.")
    parser.add_argument("--scratch-dir", type=str, default="scratch", help="If provided, put scratch files here.")
    parser.add_argument("--save-temps", type=str, help="If provided, save temporary files to the provided directory.")
    parser.add_argument("-a", "--apps", nargs="+", type=str, help="List of applications to run, case-insensitive.")
    parser.add_argument("-m", "--models", nargs="+", type=str, help="List of dest execution models to run, case-insensitive.", choices=["openmp-offload"])
    parser.add_argument("-y", "--yes-to-all", action="store_true", help="If provided, automatically answer yes to all prompts.")
    parser.add_argument("-d", "--dry", action="store_true", help="Dry run. Do not actually compile or run the code repositories.")
    parser.add_argument("-f", "--force-overwrite", action="store_true", help="If outputs are already in DB for a given prompt, then overwrite them. Default behavior is to skip existing results.")
    parser.add_argument("--hide-progress", action="store_true", help="If provided, do not show progress bar.")
    parser.add_argument("-t", "--target-path", type=str, default="targets", help="Path to the target repos including ground truths for destination models and configuration files per repo.")
    parser.add_argument("--system-config", type=str, default="config/perlmutter-config.json", help="Config for system-specific options like CUDA architecture and module load commands.")
    parser.add_argument("--log-build-output", action="store_true", help="On all builds, display the stdout of the build process.")
    parser.add_argument("--log-build-errors", action="store_true", help="On build error, display the stderr of the build process.")
    parser.add_argument("--log-run-output", action="store_true", help="On all runs, display the stdout of the run process.")
    parser.add_argument("--log-run-errors", action="store_true", help="On run error, display the stderr of the run process.")
    parser.add_argument("--log", choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"], default="INFO", type=str.upper, help="Logging level.")
    return parser.parse_args()

def gather_code_repos(args, results):
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
        where all entries are read in from the experiment_metadata.json file. '''
    code_repos = []

    # For logging what we find
    apps_found = []
    dest_models_found = []
    source_models_found = []
    llms_found = []
    prompt_strategies_found = []

    for app in os.listdir(args.translations_root):
        if args.apps and app.lower() not in [s.lower() for s in args.apps]:
            logging.debug(f"Skipping {app} because it is not in {args.apps}.")
            continue
        if app not in apps_found:
            apps_found.append(app)

        app_path = os.path.join(args.translations_root, app)

        for case_set_name in os.listdir(app_path):
            for case_name in os.listdir(os.path.join(app_path, case_set_name)):
                case_path = os.path.join(app_path, case_set_name, case_name)
                exp_meta_path = os.path.join(case_path, "experiment_metadata.json")
                repo_path = os.path.join(case_path, "repo")
                if not os.path.isfile(exp_meta_path):
                    logging.info(f"Could not find experiment_metadata.json for {case_name} under {case_path}, skipping.")
                    continue
                if not os.path.isdir(repo_path):
                    logging.error(f"Could not find repo for {case_name} under {case_path}.")
                    raise FileNotFoundError(f"Could not find repo for {case_name} under {case_path}.")

                with open(exp_meta_path, "r") as f:
                    exp_meta = json.load(f)

                    if args.models and exp_meta["dest_model"].lower() not in [s.lower() for s in args.models]:
                        logging.debug(f"Skipping {case_path}/{case_name} because dest model {exp_meta['dest_model']} not in {args.models}.")
                        continue

                    # Clean up the path in the experiment metadata
                    exp_meta["path"] = repo_path

                    prompt_strategy = exp_meta["prompt_strategy"]
                    llm_name = exp_meta["llm_name"]
                    source_model = exp_meta["source_model"]
                    dest_model = exp_meta["dest_model"]
                    output_number = exp_meta["output_number"]
                    output_path = exp_meta["path"]

                    if prompt_strategy not in prompt_strategies_found:
                        prompt_strategies_found.append(prompt_strategy)
                    if llm_name not in llms_found:
                        llms_found.append(llm_name)
                    if dest_model not in dest_models_found:
                        dest_models_found.append(dest_model)
                    if source_model not in source_models_found:
                        source_models_found.append(source_model)

                    code_repos.append(exp_meta)

                    # Check if there is an entry in the results dict matching the current repo path
                    if output_path in results["path"]:
                        logging.warning(f"Skipping duplicate code repository: {output_path}")
                    else:
                        dict_merge(results, exp_meta)
                        logging.debug(f"Found code repository: {output_path}")

    logging.info(f"Found {len(code_repos)} code repositories.")
    logging.info(f"Found apps: {apps_found}")
    logging.info(f"Found prompt strategies: {prompt_strategies_found}")
    logging.info(f"Found llms: {llms_found}")
    logging.info(f"Found dest models: {dest_models_found}")
    logging.info(f"Found source models: {source_models_found}")
    logging.debug("Code repositories:")
    logging.debug("\n" + json.dumps(code_repos, indent=4))

    return code_repos

def main():
    args = get_args()

    # Set up logging
    numeric_level = getattr(logging, args.log, None)
    if not isinstance(numeric_level, int):
        raise ValueError(f"Invalid log level: {args.log}")
    logging.basicConfig(format="%(asctime)s [%(levelname)s] -- %(message)s", level=numeric_level)

    # Warn user if using dry
    if args.dry:
        logging.warning("Running in dry mode. No code will be compiled or run.")

    # Confirm user knows the script runs LLM code
    logging.warning("This script will compile and run code generated by an LLM. " +
                    "It is recommended that you run this script in a sandboxed environment.")
    if not args.yes_to_all:
        response = await_input("Continue knowing that this script runs LLM-generated code? [y/n]: ", lambda r: r.lower() in ["y", "n", "yes", "no"])
        if response.lower() not in ["y", "yes"]:
            logging.warning("Exiting.")
            return

    # Confirm that the user doesn't want to save output if output is not provided
    if not args.output and not args.yes_to_all:
        logging.warning("No output file provided. Results will not be saved.")
        response = await_input("Continue without saving results? [y/n]: ", lambda r: r.lower() in ["y", "n", "yes", "no"])
        if response.lower() not in ["y", "yes"]:
            logging.warning("Exiting.")
            return

    # Check that force overwrite is set if output file already exists
    if args.output and os.path.exists(args.output) and not args.force_overwrite:
        logging.error(f"Output file {args.output} already exists. Use --force-overwrite to overwrite.")
        raise FileExistsError(f"Output file {args.output} already exists. Use --force-overwrite to overwrite.")

    # Check that the scratch directory exists
    scratch = os.path.abspath(args.scratch_dir)
    if not os.path.exists(scratch):
        logging.info(f"Creating scratch directory: {scratch}")
        os.makedirs(scratch)

    # Load system config
    with open(args.system_config, "r") as f:
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
        "run_stderrs_debug": []
    }

    # Gather all the code repositories
    code_repos = gather_code_repos(args, results)

    # Build and run each code repository
    with alive_bar(len(code_repos)*2, title="Building and running code repositories", max_cols=os.get_terminal_size().columns, disable=args.hide_progress) as pbar:
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
                update_results(results, results_row)

                # Copy temporary directory to save_temps if provided
                if args.save_temps:
                    tempdir_name = os.path.basename(tempdir)
                    tempdir_path = os.path.join(args.save_temps, tempdir_name)
                    logging.info(f"Saving temporary directory to {tempdir_path}.")
                    shutil.copytree(tempdir, tempdir_path)
                pbar()

    # Convert results dict to dataframe
    results_df = pd.DataFrame.from_dict(results)

    # Write the results dataframe to the output filename
    if args.output:
        logging.info(f"Writing results to {args.output}.")
        with open(args.output, "w") as f:
            json.dump(json.loads(results_df.to_json(orient="index")), f, indent=4)

if __name__ == "__main__":
    main()
