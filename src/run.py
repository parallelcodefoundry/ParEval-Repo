import logging
from subprocess import CompletedProcess

from util import run_bash, find_config

def check_output(repo_data, run_config, run_result, i):
    # Check the run output against the expected output
    if run_config["debug_type"] == "match":
        if run_result.stdout != run_config["debug_outputs"][i]:
            logging.debug(f"Output mismatch for {repo_data['app']} with model {repo_data['target_model']} test {i}.")
            logging.debug(f"Expected output: {run_config['debug_outputs'][i]}")
            logging.debug(f"Actual output: {run_result.stdout}")
            run_result.returncode = 1

def validate_binary(repo_data, run_config, run_result, i, args):
    # Validate binary executes as expected for this input
    if run_config["exec_validation_command"] != "":
        exec_validation_result = run_bash(run_config["exec_validation_command"], cwd=repo_data['path'], timeout=args.run_timeout, dry=args.dry)
        fail_text = run_config["exec_validation_fail_text"]
        if fail_text in exec_validation_result.stdout or fail_text in exec_validation_result.stderr:
            logging.debug(f"Execution validation failed for {repo_data['app']} with model {repo_data['target_model']} test {i}.")
            run_result.returncode = 1

def run_repo(repo_data, configs, result, args):
    # Find the run config for this repo
    run_config = find_config(configs, repo_data["app"], repo_data["target_model"])
    if not run_config:
        return

    # Run prerequisite commands
    #run_bash(run_config["pre_run_commands"], dry=args.dry)

    # Loop over the run commands
    run_results = []
    run_stdouts = []
    run_stderrs = []
    for i in range(0, len(run_config["run_commands_debug"])):
        run_command = run_config["run_commands_debug"][i]

        # Run the repo
        run_result = run_bash(run_command, cwd=repo_data['path'],
                              timeout=args.run_timeout, dry=args.dry)

        # Check the run output against the expected output
        check_output(repo_data, run_config, run_result, i)

        # Validate binary executes as expected for this input
        validate_binary(repo_data, run_config, run_result, i, args)

        if args.log_run_output:
            logging.info(f"Run output: {run_result.stdout}")

        # Log the run result
        if run_result.returncode != 0:
            logging.debug(f"Run failed for {repo_data['app']} with model {repo_data['target_model']} test {i}.")
            if args.log_run_errors:
                logging.info(f"Run error: {run_result.stderr}")
        else:
            logging.debug(f"Run succeeded for {repo_data['app']} with model {repo_data['target_model']} test {i}.")

        # Save the run result
        run_results.append(run_result.returncode)
        run_stdouts.append(run_result.stdout)
        run_stderrs.append(run_result.stderr)

    result["debug_results"]["run_results_debug"] = run_results
    result["debug_results"]["run_stdouts_debug"] = run_stdouts
    result["debug_results"]["run_stderrs_debug"] = run_stderrs
