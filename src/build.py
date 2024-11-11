import logging
from subprocess import CompletedProcess

from util import run_bash, find_config, list_dep_cmds

def build_repo(repo_data, system_config, args, tempdir):
    # Find the target config for this repo per dest model and app name
    target_config = find_config(repo_data["app"], repo_data["dest_model"], args.target_path)

    # Run setup commands if provided
    if "setup_commands" in target_config:
        logging.debug(f"Running setup commands for {repo_data['app']} with model {repo_data['dest_model']}.")
        setup_cmds = target_config["setup_commands"]
        setup_result = run_bash(setup_cmds, cwd=tempdir, timeout=target_config["build_timeout"], dry=args.dry)
        if setup_result.returncode != 0:
            logging.error(f"Setup failed for {repo_data['app']} with model {repo_data['dest_model']}.")
            return

    # Get cmds from system, target config
    cmds = list_dep_cmds(system_config, target_config)
    cmds.append(target_config["build_commands_debug"])

    # Build the repo
    build_result = run_bash(cmds, cwd=tempdir, timeout=target_config["build_timeout"], dry=args.dry)

    if args.log_build_output:
        logging.info(f"Build output: {build_result.stdout}")

    # Log the build result
    if build_result.returncode != 0:
        logging.debug(f"Build failed for {repo_data['app']} with model {repo_data['dest_model']}.")
        if args.log_build_errors:
            logging.info(f"Build error: {build_result.stderr}")
    else:
        logging.debug(f"Build succeeded for {repo_data['app']} with model {repo_data['dest_model']}.")

    # Create build result dict to return
    result = {}
    result["path"] = repo_data['path']
    result["build_result_debug"] = build_result.returncode
    result["build_stdout_debug"] = build_result.stdout
    result["build_stderr_debug"] = build_result.stderr

    return result
