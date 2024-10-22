import logging
from subprocess import CompletedProcess

from util import run_bash, find_config

def build_repo(repo_data, configs, result, args):
    # Find the build config for this repo
    build_config = find_config(configs, repo_data["app"], repo_data["target_model"])
    if not build_config:
        return

    # Run prerequisite commands
    run_bash(build_config["pre_build_commands"], dry=args.dry)

    # Build the repo
    build_command = build_config["build_commands_debug"]
    build_result = run_bash(build_command, cwd=repo_data['path'], timeout=args.build_timeout, dry=args.dry)

    # Log the build result
    if build_result.returncode != 0:
        logging.debug(f"Build failed for {repo_data['app']} with model {repo_data['target_model']}.")
        if args.log_build_output:
            logging.info(f"Build output: {build_result.stdout}")
        if args.log_build_errors:
            logging.info(f"Build error: {build_result.stderr}")
    else:
        logging.debug(f"Build succeeded for {repo_data['app']} with model {repo_data['target_model']}.")

    # Save the build result
    result["build_results"]["build_result_debug"] = build_result.returncode
    result["build_results"]["build_stdout_debug"] = build_result.stdout
    result["build_results"]["build_stderr_debug"] = build_result.stderr
