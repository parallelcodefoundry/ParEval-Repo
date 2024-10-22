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

# tpl imports
from tqdm import tqdm

# local imports
from util import await_input
from build import build_repo

def get_args():
    parser = ArgumentParser(description="Compile and run all the generated code repositories.")
    parser.add_argument("translations_root", type=str, help="Root directory of the generated code repositories.")
    parser.add_argument("-o", "--output", type=str, help="Output JSON file containing the results.")
    parser.add_argument("--scratch-dir", type=str, help="If provided, put scratch files here.")
    parser.add_argument("-a", "--apps", nargs="+", type=str, help="List of applications to run, case-insensitive.")
    parser.add_argument("-m", "--models", nargs="+", type=str, help="List of execution models to run, case-insensitive.", choices=["omp"])
    parser.add_argument("-y", "--yes-to-all", action="store_true", help="If provided, automatically answer yes to all prompts.")
    parser.add_argument("-d", "--dry", action="store_true", help="Dry run. Do not actually compile or run the code repositories.")
    parser.add_argument("-f", "--force-overwrite", action="store_true", help="If outputs are already in DB for a given prompt, then overwrite them. Default behavior is to skip existing results.")
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
    parser.add_argument("--log", choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"], default="INFO", type=str.upper, help="Logging level.")
    return parser.parse_args()

'''
Gather all the generated code repositories. Root directory format is:
translations_root/app/prompt_strategy-llm_name-source_model-to-target_model/output-number
Want to create list of dictionaries of the form:
{
    "app": app,
    "prompt_strategy": prompt_strategy,
    "llm_name": llm_name,
    "source_model": source_model,
    "target_model": target_model,
    "output_number": output_number,
    "path": path
}
'''
def gather_code_repos(args, results):
    code_repos = []

    # For logging what we find
    apps_found = []
    target_models_found = []
    source_models_found = []
    llms_found = []
    prompt_strategies_found = []

    for app_candidate in os.listdir(args.translations_root):
        if not args.apps or app_candidate.lower() in args.apps:
            if app_candidate not in apps_found:
                apps_found.append(app_candidate)

            app = app_candidate
            app_path = os.path.join(args.translations_root, app)

            for config in os.listdir(app_path):
                if not args.models or config.lower() in args.models:
                    logging.debug(f"Found config: {config}")
                    prompt_strategy, llm_name, source_model, _, target_model = config.split("-")

                    if prompt_strategy not in prompt_strategies_found:
                        prompt_strategies_found.append(prompt_strategy)
                    if llm_name not in llms_found:
                        llms_found.append(llm_name)
                    if target_model not in target_models_found:
                        target_models_found.append(target_model)
                    if source_model not in source_models_found:
                        source_models_found.append(source_model)

                    config_path = os.path.join(args.translations_root, app, config)

                    for output in os.listdir(config_path):
                        output_number = output.split("-")[-1]
                        output_path = os.path.join(config_path, output)

                        if not os.path.isdir(output_path):
                            logging.warning(f"Skipping non-directory {output_path}.")
                            continue

                        repo_metadata = {
                            "app": app,
                            "prompt_strategy": prompt_strategy,
                            "llm_name": llm_name,
                            "source_model": source_model,
                            "target_model": target_model,
                            "output_number": output_number,
                            "path": output_path
                        }
                        code_repos.append(repo_metadata)

                        # Hash the metadata to use as a key in the results dict
                        hashcode = hash(json.dumps(repo_metadata, sort_keys=True))
                        logging.debug(f"Data to hash: {json.dumps(repo_metadata, sort_keys=True)}")
                        logging.debug(f"Hashcode to save: {hashcode}")

                        if hashcode in results:
                            logging.warning(f"Skipping duplicate code repository: {output_path}")
                        else:
                            results[hashcode] = copy.deepcopy(repo_metadata)
                            results[hashcode]["build_results"] = {}
                            results[hashcode]["debug_results"] = {}
                            results[hashcode]["perf_results"] = {}
                            logging.debug(f"Found code repository: {output_path}")

    logging.info(f"Found {len(code_repos)} code repositories.")
    logging.info(f"Found apps: {apps_found}")
    logging.info(f"Found prompt strategies: {prompt_strategies_found}")
    logging.info(f"Found llms: {llms_found}")
    logging.info(f"Found target models: {target_models_found}")
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
        response = await_input("Continue? [y/n]: ", lambda r: r.lower() in ["y", "n", "yes", "no"])
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

    # Load build and run configs
    with open(args.build_config, "r") as f:
        build_configs = json.load(f)

    with open(args.run_config, "r") as f:
        run_configs = json.load(f)

    # Create empty dict of (hashcode,dict) pairs for results dicts
    results = {}

    # Gather all the code repositories
    code_repos = gather_code_repos(args, results)

    # Build each code repository
    for code_repo in tqdm(code_repos, desc="Building code repositories", disable=args.hide_progress):
        logging.debug(f"Building code repository: {code_repo['path']}")
        logging.debug(f"Code repo metadata to hash: {json.dumps(code_repo, sort_keys=True)}")
        hashcode = hash(json.dumps(code_repo, sort_keys=True))
        build_repo(code_repo, build_configs, results[hashcode], args)

    # Filter out results that are already in the output
    if args.output and os.path.exists(args.output):
        if args.force_overwrite:
            logging.info("Overwriting existing results.")
        else:
            logging.info("Filtering out results that are already in the output.")
            with open(args.output, "r") as f:
                existing_results = json.load(f)
            results = {k: v for k, v in results.items() if k not in existing_results}

    # Write the results to the output filename
    if args.output:
        with open(args.output, "w") as f:
            json.dump(results, f, indent=4)

if __name__ == "__main__":
    main()
