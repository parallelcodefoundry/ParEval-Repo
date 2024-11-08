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
import copy
import contextlib

# tpl imports
from tqdm import tqdm
import pandas as pd
import tempfile

# local imports
from util import await_input, setup_tempdir, meta_to_arr, update_results
from build import build_repo
from run import run_repo

def get_args():
    parser = ArgumentParser(description="Compile and run all the generated code repositories.")
    parser.add_argument("translations_root", type=str, help="Root directory of the generated code repositories.")
    parser.add_argument("-o", "--output", type=str, help="Output JSON file containing the results.")
    parser.add_argument("--scratch-dir", type=str, default="scratch", help="If provided, put scratch files here.")
    parser.add_argument("--save-temps", action="store_true", help="If provided, save temporary files.")
    parser.add_argument("-a", "--apps", nargs="+", type=str, help="List of applications to run, case-insensitive.")
    parser.add_argument("-m", "--models", nargs="+", type=str, help="List of dest execution models to run, case-insensitive.", choices=["omp"])
    parser.add_argument("-y", "--yes-to-all", action="store_true", help="If provided, automatically answer yes to all prompts.")
    parser.add_argument("-d", "--dry", action="store_true", help="Dry run. Do not actually compile or run the code repositories.")
    parser.add_argument("-f", "--force-overwrite", action="store_true", help="If outputs are already in DB for a given prompt, then overwrite them. Default behavior is to skip existing results.")
    parser.add_argument("--hide-progress", action="store_true", help="If provided, do not show progress bar.")
    parser.add_argument("--build-only", action="store_true", help="If provided, only build the code repositories, do not run.")
    parser.add_argument("--run-only", action="store_true", help="If provided, only run the code repositories, do not build.")
    parser.add_argument("-t", "--target-path", type=str, default="targets", help="Path to the target repos including ground truths for destination models and configuration files per repo.")
    parser.add_argument("--system-config", type=str, default="config/perlmutter-config.json", help="Config for system-specific options like CUDA architecture and module load commands.")
    parser.add_argument("--log-build-output", action="store_true", help="On all builds, display the stdout of the build process.")
    parser.add_argument("--log-build-errors", action="store_true", help="On build error, display the stderr of the build process.")
    parser.add_argument("--log-run-output", action="store_true", help="On all runs, display the stdout of the run process.")
    parser.add_argument("--log-run-errors", action="store_true", help="On run error, display the stderr of the run process.")
    parser.add_argument("--log", choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"], default="INFO", type=str.upper, help="Logging level.")
    return parser.parse_args()

'''
Gather all the generated code repositories. Root directory format is:
translations_root/app/case-name/output-number/, including meta.json and repo/
under each output-number directory. repo/ contains the generated code. Want to
create list of dictionaries of the form:
{
    "app": app,
    "prompt_strategy": prompt_strategy,
    "llm_name": llm_name,
    "source_model": source_model,
    "dest_model": dest_model,
    "output_number": output_number,
    "path": path
}
where all entries are read in from the meta.json file.
'''
def gather_code_repos(args, results):
    code_repos = []

    # For logging what we find
    apps_found = []
    dest_models_found = []
    source_models_found = []
    llms_found = []
    prompt_strategies_found = []

    for app in os.listdir(args.translations_root):
        if args.apps and app.lower() not in args.apps:
            logging.debug(f"Skipping {app} because it is not in {args.apps}.")
            continue
        if app not in apps_found:
            apps_found.append(app)

        app_path = os.path.join(args.translations_root, app)

        for case_set_name in os.listdir(app_path):
            for case_name in os.listdir(os.path.join(app_path, case_set_name)):
                case_path = os.path.join(app_path, case_set_name, case_name)
                meta_path = os.path.join(case_path, "meta.json")
                repo_path = os.path.join(case_path, "repo")
                if not os.path.isfile(meta_path):
                    logging.error(f"Could not find meta.json for {case_name} under {case_path}.")
                    raise FileNotFoundError(f"Could not find meta.json for {case_name} under {case_path}.")
                if not os.path.isdir(repo_path):
                    logging.error(f"Could not find repo for {case_name} under {case_path}.")
                    raise FileNotFoundError(f"Could not find repo for {case_name} under {case_path}.")

                with open(meta_path, "r") as f:
                    meta = json.load(f)

                    if args.models and meta["dest_model"] not in args.models:
                        logging.debug(f"Skipping {case_path}/{case_name} because dest model {meta['dest_model']} not in {args.models}.")
                        continue

                    prompt_strategy = meta["prompt_strategy"]
                    llm_name = meta["llm_name"]
                    source_model = meta["source_model"]
                    dest_model = meta["dest_model"]
                    output_number = meta["output_number"]
                    output_path = meta["path"]

                    if prompt_strategy not in prompt_strategies_found:
                        prompt_strategies_found.append(prompt_strategy)
                    if llm_name not in llms_found:
                        llms_found.append(llm_name)
                    if dest_model not in dest_models_found:
                        dest_models_found.append(dest_model)
                    if source_model not in source_models_found:
                        source_models_found.append(source_model)

                    code_repos.append(meta)

                    # Check if there is an entry in the results dataframe matching the current repo path
                    if output_path in results["path"].values:
                        logging.warning(f"Skipping duplicate code repository: {output_path}")
                    else:
                        results.loc[len(results)] = meta_to_arr(meta)
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

    # Check that the scratch directory exists
    scratch = os.path.abspath(args.scratch_dir)
    if not os.path.exists(scratch):
        logging.info(f"Creating scratch directory: {scratch}")
        os.makedirs(scratch)

    # Load system config
    with open(args.system_config, "r") as f:
        system_config = json.load(f)
    logging.debug(f"Loaded system config: {system_config}")

    # Create empty dataframe to store results with columns for each metadata field
    results = pd.DataFrame(columns=["app",
                                    "prompt_strategy",
                                    "llm_name",
                                    "source_model",
                                    "dest_model",
                                    "output_number",
                                    "path",
                                    "build_result_debug",
                                    "build_stdout_debug",
                                    "build_stderr_debug",
                                    "run_results_debug",
                                    "run_exec_checks_debug",
                                    "run_stdouts_debug",
                                    "run_stderrs_debug"])

    # Gather all the code repositories
    code_repos = gather_code_repos(args, results)

    # Build and run each code repository
    pbar = tqdm(total=len(code_repos)*2, desc="Building and running code repositories", disable=args.hide_progress)
    for code_repo in code_repos:
        # Want temporary directory to not be cleaned up if user requests it, but
        # delete option in TemporaryDirectory is only available in Python 3.12+
        with (contextlib.nullcontext(tempfile.mkdtemp(dir=scratch))
              if args.save_temps
              else tempfile.TemporaryDirectory(dir=scratch)
              ) as tempdir:
            logging.debug(f"Temporary directory created: {tempdir}")
            setup_tempdir(tempdir, code_repo)

            logging.debug(f"Building code repository: {code_repo['path']}")
            loc_results = build_repo(code_repo, system_config, args, tempdir)
            update_results(results, loc_results)
            pbar.update(1)

            logging.debug(f"Running code repository: {code_repo['path']}")
            loc_results = run_repo(code_repo, system_config, args, tempdir)
            update_results(results, loc_results)
            pbar.update(1)
    pbar.close()

    # Filter out results that are already in the output
    if args.output and os.path.exists(args.output):
        if args.force_overwrite:
            logging.info("Overwriting existing results.")
        else:
            logging.info("Filtering out results that are already in the output.")
            with open(args.output, "r") as f:
                existing_results = json.load(f)
            results = {k: v for k, v in results.items() if k not in existing_results}

    # Write the results DataFrame to the output filename
    if args.output:
        logging.info(f"Writing results to {args.output}.")
        with open(args.output, "w") as f:
            json.dump(json.loads(results.to_json(orient="index")), f, indent=4)

if __name__ == "__main__":
    main()
