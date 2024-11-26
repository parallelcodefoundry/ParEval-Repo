import logging
from subprocess import CompletedProcess

from util import run_bash, find_config, list_dep_cmds

def check_output(repo_data, target_config, run_result, i):
    ''' Check the run output against the expected output '''
    if target_config["debug_type"] == "match":
        if target_config["debug_outputs"][i] not in run_result.stdout:
            logging.debug(f"Output mismatch for {repo_data['app']} with model {repo_data['dest_model']} test {i}.")
            logging.debug(f"Expected output to contain: {target_config['debug_outputs'][i]}")
            logging.debug(f"Actual output: {run_result.stdout}")
            return 1
    return 0

def check_exec(repo_data, target_config, system_config, i, args, tempdir):
    ''' Check that binary executes as expected for this input '''
    if system_config["exec_check"] != "":
        cmds = list_dep_cmds(system_config, target_config)
        exc_cmd = [system_config["exec_check"] + " " + target_config["run_commands_perf"][0]]
        exec_check_result = run_bash(cmds + exc_cmd, cwd=tempdir, timeout=target_config["run_timeout"], dry=args.dry)
        fail_text = system_config["exec_check_fail_text"]
        if fail_text == "":
            logging.error(f"exec_check_fail_text not specified in {args.system_config}.")
            raise ValueError(f"exec_check_fail_text not specified in {args.system_config}.")
        if fail_text in exec_check_result.stdout or fail_text in exec_check_result.stderr:
            logging.debug(f"Execution check failed for {repo_data['app']} with model {repo_data['dest_model']} test {i}.")
            return 1
    else:
        logging.debug(f"No execution check specified in {args.system_config}.")
    return 0

def run_repo(repo_data, system_config, args, tempdir):
    # Find the target config for this repo per dest model and app name
    target_config = find_config(repo_data["app"], repo_data["dest_model"], args.target_path)

    # Get dep cmds from system, target config
    cmds = list_dep_cmds(system_config, target_config)

    # Loop over the run cmds
    run_results = []
    run_exec_checks = []
    run_stdouts = []
    run_stderrs = []
    for i in range(0, len(target_config["run_commands_debug"])):
        run_cmd = [target_config["run_commands_debug"][i]]

        # Run the repo
        run_result = run_bash(cmds + run_cmd, cwd=tempdir,
                              timeout=target_config["run_timeout"], dry=args.dry)

        # Check the run output against the expected output
        run_result.returncode = check_output(repo_data, target_config, run_result, i)

        # Validate binary executes as expected for this input
        exec_check = check_exec(repo_data, target_config, system_config, i, args, tempdir)

        if args.log_run_output:
            logging.info(f"Run output: {run_result.stdout}")

        # Log the run result
        if run_result.returncode != 0:
            logging.debug(f"Run failed for {repo_data['app']} with model {repo_data['dest_model']} test {i}.")
            if args.log_run_errors:
                logging.info(f"Run error: {run_result.stderr}")
        else:
            logging.debug(f"Run succeeded for {repo_data['app']} with model {repo_data['dest_model']} test {i}.")

        # Save the run result
        run_results.append(run_result.returncode)
        run_exec_checks.append(exec_check)
        run_stdouts.append(run_result.stdout)
        run_stderrs.append(run_result.stderr)

    # Create run result dict to return
    result = {}
    result["path"] = repo_data['path']
    result["run_results_debug"] = run_results
    result["run_exec_checks_debug"] = run_exec_checks
    result["run_stdouts_debug"] = run_stdouts
    result["run_stderrs_debug"] = run_stderrs

    return result
