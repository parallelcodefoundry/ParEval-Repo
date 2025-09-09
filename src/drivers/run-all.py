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
from typing import Dict, List, Optional, Any

# tpl imports
from alive_progress import alive_bar
import pandas as pd

# local imports
from util import await_input, setup_tempdir, dict_merge, empty_results_dict, empty_other_keys_at_idx
from build import build_repo
from run import run_repo, make_skip_run_result
from util import FileSystemHelper

def get_args() -> ArgumentParser:
    ''' Get command line arguments.
    '''
    parser = ArgumentParser(description="Compile and run all the generated code repositories.")
    parser.add_argument("translations_root", type=str,
                        help="Root directory of the generated code repositories.")
    parser.add_argument("--outputs-tarball", action="store_true",
                        help="If provided, untar tarballs in the translations root directory.")
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
    parser.add_argument("--skip-build-swap", action="store_true",
                        help="If provided, do not retry failed builds with ground truth build system file.")
    parser.add_argument("-y", "--yes-to-all", action="store_true",
                        help="If provided, automatically answer yes to all prompts.")
    parser.add_argument("-d", "--dry", action="store_true",
                        help="Dry run. Do not actually compile or run the code repositories.")
    parser.add_argument("-s", "--skip-repeats", action="store_true",
                        help="If provided, skip code repositories that have already been run, " +
                        "appending to the output file. Takes precedence over --force-overwrite.")
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


def check_app_filter(app: str, args: ArgumentParser) -> bool:
    """Check if app passes the filter criteria."""
    if args.apps and app.lower() not in [s.lower() for s in args.apps]:
        logging.debug(f"Skipping {app} because it is not in {args.apps}.")
        return False
    return True


def check_model_filter(dest_model: str, args: ArgumentParser) -> bool:
    """Check if dest_model passes the filter criteria."""
    if args.models and dest_model.lower() not in [s.lower() for s in args.models]:
        logging.debug(f"Skipping {dest_model} because it is not in {args.models}.")
        return False
    return True


def check_prompt_strategy_filter(prompt_strategy: str, args: ArgumentParser) -> bool:
    """Check if prompt_strategy passes the filter criteria."""
    if args.prompt_strategies and prompt_strategy.lower() not in \
       [s.lower() for s in args.prompt_strategies]:
        logging.debug(f"Skipping {prompt_strategy} because it is not in {args.prompt_strategies}.")
        return False
    return True


def normalize_dest_model(exp_meta: Dict[str, str]) -> str:
    """Normalize dest_model field (omp -> openmp-offload)."""
    dest_model = exp_meta["dest_model"]
    if dest_model == "omp":
        dest_model = "openmp-offload"
        exp_meta["dest_model"] = "openmp-offload"
    return dest_model


def normalize_prompt_strategy(exp_meta: Dict[str, str]) -> str:
    """Normalize prompt_strategy field (convert list to string)."""
    prompt_strategy = exp_meta["prompt_strategy"]
    if isinstance(prompt_strategy, list):
        prompt_strategy = "-".join(prompt_strategy)
    return prompt_strategy


def update_metadata_found(exp_meta: Dict[str, str], metadata_found: Dict[str, List]) -> None:
    """Update the metadata_found tracking dictionary."""
    app = exp_meta["app"]
    dest_model = exp_meta["dest_model"]
    prompt_strategy = exp_meta["prompt_strategy"]
    llm_name = exp_meta["llm_name"]
    source_model = exp_meta["source_model"]

    if app not in metadata_found["apps_found"]:
        metadata_found["apps_found"].append(app)
    if dest_model not in metadata_found["dest_models_found"]:
        metadata_found["dest_models_found"].append(dest_model)
    if prompt_strategy not in metadata_found["prompt_strategies_found"]:
        metadata_found["prompt_strategies_found"].append(prompt_strategy)
    if llm_name not in metadata_found["llms_found"]:
        metadata_found["llms_found"].append(llm_name)
    if source_model not in metadata_found["source_models_found"]:
        metadata_found["source_models_found"].append(source_model)


def parse_metadata(args: ArgumentParser, exp_meta: Dict[str, str],
                   metadata_found: Dict[str, List]) -> Optional[Dict[str, str]]:
    """Parse the metadata from the experiment_metadata.json file."""
    app = exp_meta["app"]
    if not check_app_filter(app, args):
        return None

    dest_model = normalize_dest_model(exp_meta)
    if not check_model_filter(dest_model, args):
        return None

    prompt_strategy = normalize_prompt_strategy(exp_meta)
    if not check_prompt_strategy_filter(prompt_strategy, args):
        return None

    # Update tracking
    update_metadata_found(exp_meta, metadata_found)
    return exp_meta


def log_metadata_found(metadata_found: Dict[str, List],
                       code_repos: List[Dict[str, str]]):
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


def initialize_metadata_tracking() -> Dict[str, List]:
    """Initialize the metadata tracking dictionary."""
    return {
        "apps_found": [],
        "dest_models_found": [],
        "source_models_found": [],
        "llms_found": [],
        "prompt_strategies_found": [],
    }


def process_experiment_metadata_file(exp_meta_path: str, dirpath: str,
                                   args: ArgumentParser, metadata_found: Dict[str, List],
                                   results: Dict[str, List], skip_repeats: bool) -> Optional[Dict[str, str]]:
    """Process a single experiment_metadata.json file."""
    repo_path = os.path.join(dirpath, "repo")

    if not os.path.isfile(exp_meta_path):
        logging.info(f"Could not find experiment_metadata.json under {dirpath}, skipping.")
        return None

    if not os.path.isdir(repo_path):
        logging.error(f"Could not find repo under {dirpath}.")
        raise FileNotFoundError(f"Could not find repo under {dirpath}.")

    with open(exp_meta_path, "r", encoding="utf-8") as f:
        exp_meta = json.load(f)

    # Clean up the path in the experiment metadata
    exp_meta["path"] = repo_path

    # Parse the metadata
    exp_meta = parse_metadata(args, exp_meta, metadata_found)

    # Go to next iter if parse_metadata decided to skip due to filters
    if exp_meta is None:
        return None

    # Check if there is an entry in the results dict matching the current repo path
    output_path = exp_meta["path"]
    if output_path in results["path"] and skip_repeats:
        idx = results["path"].index(output_path)
        # Only skip if result found has complete data
        if results["run_stdouts_debug"][idx] is not None:
            logging.debug(f"Skipping duplicate code repository: {output_path}")
            return "skip"
        # Need to set non-metadata keys' values to None if found incomplete data
        empty_other_keys_at_idx(results, exp_meta, idx)
    else:
        # Only need to add metadata to the results if not present
        dict_merge(results, exp_meta)

    logging.debug(f"Found code repository: {output_path}")
    return exp_meta


def gather_code_repos(args: ArgumentParser, results: Dict[str, List], skip_repeats: bool) \
    -> List[Dict[str, str]]:
    """Gather all the generated code repositories.

    Root directory format is: translations_root/app/case-name/output-number/,
    including experiment_metadata.json and repo/ under each output-number directory.
    repo/ contains the generated code. Creates list of dictionaries with metadata
    from experiment_metadata.json file.

    If skip_repeats is set, skip code repositories that have already been run.
    """
    code_repos = []
    metadata_found = initialize_metadata_tracking()
    num_skipped = 0

    for dirpath, _, filenames in os.walk(args.translations_root):
        for filename in filenames:
            if filename == "experiment_metadata.json":
                exp_meta_path = os.path.join(dirpath, "experiment_metadata.json")
                result = process_experiment_metadata_file(
                    exp_meta_path, dirpath, args, metadata_found, results, skip_repeats)

                if result == "skip":
                    num_skipped += 1
                elif result is not None:
                    code_repos.append(result)

    if num_skipped > 0:
        logging.info(f"Skipped {num_skipped} code repositories that have already been run.")

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


def setup_logging(log_level: str) -> None:
    """Set up logging configuration."""
    numeric_level = getattr(logging, log_level, None)
    if not isinstance(numeric_level, int):
        raise ValueError(f"Invalid log level: {log_level}")
    logging.basicConfig(format="%(asctime)s [%(levelname)s] -- %(message)s",
                        level=numeric_level)


def validate_directories(translations_root: str, scratch_dir: str) -> str:
    """Validate and create necessary directories."""
    # Make sure the translations root directory exists
    if not os.path.exists(translations_root):
        raise FileNotFoundError(f"Translations root directory {translations_root} does not exist.")

    # Check that the scratch directory exists
    scratch = os.path.abspath(scratch_dir)
    if not os.path.exists(scratch):
        logging.info(f"Creating scratch directory: {scratch}")
        os.makedirs(scratch)

    return scratch


def handle_dry_run_warning(dry: bool) -> None:
    """Handle dry run warning."""
    if dry:
        logging.warning("Running in dry mode. No code will be compiled or run!")


def confirm_llm_code_execution(yes_to_all: bool) -> None:
    """Confirm user knows the script runs LLM code."""
    logging.warning("This script will compile and run code generated by an LLM. " +
                    "It is recommended that you run this script in a sandboxed environment.")
    if not yes_to_all:
        response = await_input("Continue knowing that this script runs LLM-generated code? [y/n]: ",
                               lambda r: r.lower() in ["y", "n", "yes", "no"])
        if response.lower() not in ["y", "yes"]:
            logging.warning("Exiting.")
            sys.exit(0)


def handle_output_file_conflicts(output_file: str, force_overwrite: bool, skip_repeats: bool) -> None:
    """Handle conflicts with existing output files."""
    if os.path.exists(output_file):
        if not (force_overwrite or skip_repeats):
            raise FileExistsError(f"Output file {output_file} already exists. " +
                                  "Use --force-overwrite to overwrite or " +
                                  "--skip-repeats to skip existing runs and append.")
        if force_overwrite and not skip_repeats:
            logging.warning(f"Output file {output_file} already exists. Overwriting.")
            os.remove(output_file)


def startup_from_args(args: ArgumentParser) -> os.PathLike:
    """Startup from command line arguments."""
    # Set up logging
    setup_logging(args.log)

    # Validate directories
    scratch = validate_directories(args.translations_root, args.scratch_dir)

    # Handle warnings and confirmations
    handle_dry_run_warning(args.dry)
    confirm_llm_code_execution(args.yes_to_all)

    # Handle output file conflicts
    handle_output_file_conflicts(args.output, args.force_overwrite, args.skip_repeats)

    return scratch


def save_temps(tempdir: os.PathLike, args: ArgumentParser,
               results_row: Dict[str, str]):
    """Save temporary files to the provided directory."""
    FileSystemHelper.save_temps(tempdir, args.save_temps, results_row)


def update_results(results, results_row, output):
    ''' Update the results dict of lists with an individual results dictionary,
        matching based on path.
    '''
    # Find the row in the results dict that matches the path
    path = results_row["path"]
    if path not in results["path"]:
        raise ValueError(f"Path {path} not found in results.")
    row_idx = results["path"].index(path)

    # Update the results dict with the results row
    for key, value in results_row.items():
        if key in results:
            if results[key][row_idx] is None or results_row["ground_truth_build"]:
                results[key][row_idx] = value
            elif results[key][row_idx] != value:
                raise ValueError(f"Key already has a non-matching value in results, " +
                                 f"{results[key][row_idx]} != {value}.")
        else:
            raise ValueError(f"Key {key} not found in results.")

    # Convert results dict to dataframe
    results_df = pd.DataFrame.from_dict(results)

    # Back up the existing output file if it exists before writing
    if os.path.exists(output):
        logging.debug(f"Backing up {output} to {output}.bak.")
        shutil.copyfile(output, output + ".bak")

    # Write the results to the output file
    logging.info(f"Writing results to {output}.")
    with open(output, "w", encoding="utf-8") as f:
        json.dump(json.loads(results_df.to_json(orient="index")), f, indent=4)

    # Remove the backup file if the write was successful
    if os.path.exists(output + ".bak"):
        os.remove(output + ".bak")


def handle_build_success(code_repo: Dict[str, str], system_config: Dict[str, str],
                        args: ArgumentParser, tempdir: str, ground_truth_build: bool,
                        results: Dict[str, List], output: os.PathLike, pbar: alive_bar) -> None:
    """Handle successful build by running the code repository."""
    logging.debug(f"Running code repository: {code_repo['path']}")
    results_row_run = run_repo(code_repo, system_config, args, tempdir)
    results_row_run["ground_truth_build"] = ground_truth_build

    # Copy temporary directory to save_temps if provided
    if args.save_temps:
        save_temps(tempdir, args, results_row_run)

    update_results(results, results_row_run, output)


def handle_build_failure(code_repo: Dict[str, str], ground_truth_build: bool,
                        args: ArgumentParser, results: Dict[str, List],
                        output: os.PathLike) -> None:
    """Handle build failure by creating skip result."""
    logging.debug(f"Skipping run for {code_repo['path']} due to build failure.")
    results_row_run = make_skip_run_result(code_repo)
    results_row_run["ground_truth_build"] = ground_truth_build
    update_results(results, results_row_run, output)


def process_build_results(results_row_build: Dict[str, str], ground_truth_build: bool) -> None:
    """Process build results and save debug information."""
    if not ground_truth_build:
        # Save the stdout and stderr of the build process with the original makefile
        # for debugging since ground truth build will overwrite otherwise
        results_row_build["gen_build_stdout_debug"] = results_row_build["build_stdout_debug"]
        results_row_build["gen_build_stderr_debug"] = results_row_build["build_stderr_debug"]


def process_repo(code_repo: Dict[str, str], results: Dict[str, List],
                 system_config: Dict[str, str], args: ArgumentParser,
                 scratch: os.PathLike, pbar: alive_bar,
                 output: os.PathLike,
                 ground_truth_build: Optional[bool] = False) -> Dict[str, str]:
    """Build and run the code repository."""
    results_row_build = {}

    with tempfile.TemporaryDirectory(dir=scratch) as tempdir:
        logging.debug(f"Temporary directory created: {tempdir}")
        setup_tempdir(tempdir, code_repo)

        # Build the repository
        logging.debug(f"Building code repository: {code_repo['path']}")
        results_row_build = build_repo(code_repo, system_config, args, tempdir,
                                       ground_truth_build=ground_truth_build)

        # Process build results
        process_build_results(results_row_build, ground_truth_build)
        update_results(results, results_row_build, output)
        pbar()

        # Handle build success or failure
        if results_row_build["build_result_debug"] == 0:
            handle_build_success(code_repo, system_config, args, tempdir,
                               ground_truth_build, results, output, pbar)
        elif ground_truth_build or args.skip_build_swap:
            handle_build_failure(code_repo, ground_truth_build, args, results, output)

        # Clean up temporary directory
        FileSystemHelper.cleanup_tempdir(tempdir)
        pbar()

    return results_row_build


def load_system_config(system_config_path: str) -> Dict[str, Any]:
    """Load system configuration from file."""
    with open(system_config_path, "r", encoding="utf-8") as f:
        system_config = json.load(f)
    logging.debug(f"Loaded system config: {system_config}")
    return system_config


def load_existing_results(output_file: str) -> Dict[str, List]:
    """Load existing results from file if skip_repeats is enabled."""
    with open(output_file, "r", encoding="utf-8") as f:
        results = json.loads(pd.read_json(f, orient="index").to_json(orient="columns"))
        # convert back to dict of lists
        results = {k: list(v.values()) for k, v in results.items()}
    logging.info(f"Loaded {len(results['path'])} results from {output_file}.")
    return results


def initialize_results(args: ArgumentParser) -> Dict[str, List]:
    """Initialize results dictionary from disk or create empty one."""
    if args.skip_repeats and os.path.exists(args.output):
        return load_existing_results(args.output)
    else:
        return empty_results_dict()


def process_code_repositories(code_repos: List[Dict[str, str]], results: Dict[str, List],
                            system_config: Dict[str, Any], args: ArgumentParser,
                            scratch: os.PathLike) -> None:
    """Process all code repositories with progress bar."""
    max_cols = safe_get_cols()
    items = len(code_repos) * (2 if args.skip_build_swap else 4)

    with alive_bar(items,
                   title="Building and running code repositories",
                   max_cols=max_cols, disable=args.hide_progress) as pbar:
        for code_repo in code_repos:
            results_row = process_repo(code_repo, results, system_config, args,
                                       scratch, pbar, args.output)

            if not args.skip_build_swap:
                if results_row["build_result_debug"] != 0:
                    process_repo(code_repo, results, system_config, args,
                                 scratch, pbar, args.output,
                                 ground_truth_build=True)
                else:
                    pbar()
                    pbar()


def main():
    """Main function."""
    args = get_args()

    # Startup from command line arguments
    scratch = startup_from_args(args)

    # Extract tarballs if requested
    if args.outputs_tarball:
        logging.info("Extracting tarballs in translations root directory.")
        extract_tarballs(args.translations_root, remove_tarball=False)

    # Load system config
    system_config = load_system_config(args.system_config)

    # Initialize results
    results = initialize_results(args)

    # Gather all the code repositories
    code_repos = gather_code_repos(args, results, args.skip_repeats)

    # Process all code repositories
    process_code_repositories(code_repos, results, system_config, args, scratch)

if __name__ == "__main__":
    main()
