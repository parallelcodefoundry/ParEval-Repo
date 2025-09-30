import logging
from typing import Dict, Any

from util import run_bash, find_config, list_dep_cmds, ResultBuilder, LoggingHelper, ConfigHelper


def run_setup(repo_data: Dict[str, str], target_config: Dict[str, Any],
              args: Any, tempdir: str):
    """Run setup commands for the repo."""
    logging.debug(f"Running setup commands for {repo_data['app']} with model "
                 f"{repo_data['dest_model']}.")
    setup_cmds = target_config["setup_commands"]
    setup_result = run_bash(setup_cmds, cwd=tempdir,
                            timeout=target_config["build_timeout"],
                            dry=args.dry)
    return setup_result


def handle_setup_failure(repo_data: Dict[str, str], setup_result,
                        ground_truth_build: bool) -> Dict[str, Any]:
    """Handle setup command failure."""
    logging.error(f"Setup failed for {repo_data['app']} with model "
                 f"{repo_data['dest_model']}.")
    result = ResultBuilder.build_build_result(repo_data, setup_result, ground_truth_build)
    result["gen_build_stdout_debug"] = result["build_stdout_debug"]
    result["gen_build_stderr_debug"] = result["build_stderr_debug"]
    return result


def execute_build_commands(repo_data: Dict[str, str], system_config: Dict[str, Any],
                          target_config: Dict[str, Any], args: Any, tempdir: str,
                          ground_truth_build: bool) -> Dict[str, Any]:
    """Execute the main build commands."""
    # Get cmds from system, target config
    cmds = list_dep_cmds(system_config, target_config)
    cmds.append(target_config["build_commands_debug"])

    # If ground truth build, replace existing build file
    if ground_truth_build:
        ConfigHelper.prepare_ground_truth_build(args, tempdir, target_config)

    # Build the repo
    build_result = run_bash(cmds, cwd=tempdir,
                            timeout=target_config["build_timeout"],
                            dry=args.dry,
                            name="build")

    if args.log_build_output:
        logging.info(f"Build output: {build_result.stdout}")

    # Log the build result
    LoggingHelper.log_build_result(repo_data, build_result, args)

    # Create build result dict to return
    return ResultBuilder.build_build_result(repo_data, build_result, ground_truth_build)


def build_repo(repo_data: Dict[str, str], system_config: Dict[str, Any],
               args: Any, tempdir: str, ground_truth_build: bool = False) -> Dict[str, Any]:
    """Build the repo in the given path with the given config."""
    # Find the target config for this repo per dest model and app name
    target_config = find_config(repo_data["app"], repo_data["dest_model"],
                                args.target_path)

    # Run setup commands if provided
    if "setup_commands" in target_config:
        setup_result = run_setup(repo_data, target_config, args, tempdir)
        if setup_result.returncode != 0:
            return handle_setup_failure(repo_data, setup_result, ground_truth_build)

    # Execute main build commands
    return execute_build_commands(repo_data, system_config, target_config,
                                 args, tempdir, ground_truth_build)
