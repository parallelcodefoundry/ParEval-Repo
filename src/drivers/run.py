import logging
from typing import Dict, Any, List

from util import run_bash, find_config, list_dep_cmds, ResultBuilder, LoggingHelper, ValidationHelper, ResultsManager


def execute_single_run(repo_data: Dict[str, str], target_config: Dict[str, Any],
                      system_config: Dict[str, Any], args: Any, tempdir: str,
                      test_index: int, cmds: List[str]) -> tuple:
    """Execute a single run command and return the results."""
    run_cmd = [target_config["run_commands_debug"][test_index]]

    # Run the repo
    run_result = run_bash(cmds + run_cmd, cwd=tempdir,
                          timeout=target_config["run_timeout"], dry=args.dry,
                          name="run")

    # Check the run output against the expected output
    run_result.returncode = ValidationHelper.check_output(
        repo_data, target_config, run_result, test_index)

    # Validate binary executes as expected for this input
    exec_check = ValidationHelper.check_exec(
        repo_data, target_config, system_config, test_index, args, tempdir)

    if args.log_run_output:
        logging.info(f"Run output: {run_result.stdout}")

    # Log the run result
    LoggingHelper.log_run_result(repo_data, run_result, args, test_index)

    return run_result, exec_check


def run_repo(repo_data: Dict[str, str], system_config: Dict[str, Any],
             args: Any, tempdir: str) -> Dict[str, Any]:
    """Run the repo with the given configuration."""
    # Find the target config for this repo per dest model and app name
    target_config = find_config(repo_data["app"], repo_data["dest_model"], args.target_path)

    # Get dep cmds from system, target config
    cmds = list_dep_cmds(system_config, target_config)

    # Initialize result lists
    run_results = []
    run_exec_checks = []
    run_stdouts = []
    run_stderrs = []

    # Loop over the run cmds
    for i in range(len(target_config["run_commands_debug"])):
        run_result, exec_check = execute_single_run(
            repo_data, target_config, system_config, args, tempdir, i, cmds)

        # Save the run result
        ResultsManager.save_run_result(
            run_result, exec_check, run_results, run_exec_checks,
            run_stdouts, run_stderrs)

    # Create run result dict to return
    return ResultBuilder.build_run_result(
        repo_data, run_results, run_exec_checks, run_stdouts, run_stderrs)


def make_skip_run_result(repo_data: Dict[str, str]) -> Dict[str, Any]:
    """Create a result dict for a skipped run."""
    return ResultBuilder.build_skip_run_result(repo_data)
