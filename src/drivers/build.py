import logging
from subprocess import CompletedProcess
import os
import shutil

from util import run_bash, find_config, list_dep_cmds


def run_setup(repo_data, target_config, args, tempdir):
    ''' Run setup commands for the repo.
    '''
    logging.debug(f"Running setup commands for {repo_data['app']} with model " +
                  f"{repo_data['dest_model']}.")
    setup_cmds = target_config["setup_commands"]
    setup_result = run_bash(setup_cmds, cwd=tempdir,
                            timeout=target_config["build_timeout"],
                            dry=args.dry)
    return setup_result


def prepare_result_dict(repo_data, run_result, ground_truth_build):
    ''' Create a result dict from a bash run result.
    '''
    result = {}
    result["path"] = repo_data["path"]
    result["ground_truth_build"] = ground_truth_build
    result["build_result_debug"] = run_result.returncode
    result["build_stdout_debug"] = run_result.stdout
    result["build_stderr_debug"] = run_result.stderr
    return result


def prepare_ground_truth_build(args, tempdir, target_config):
    ''' Rewrite the existing build file with the ground truth build file.
    '''
    build_fname = target_config["build_filename"]
    gen_build_fname = os.path.join(tempdir, build_fname)
    true_build_fname = os.path.join(os.path.dirname(os.path.dirname(args.target_path)),
                                    target_config["path"],
                                    build_fname)
    logging.debug(f"Rewriting build file {gen_build_fname} with ground " +
                  f"truth build file {true_build_fname}.")
    shutil.copyfile(true_build_fname, gen_build_fname)


def log_build_result(repo_data, build_result, args):
    ''' Log the build result.
    '''
    if build_result.returncode != 0:
        logging.debug(f"Build failed for {repo_data['app']} with model " +
                      f"{repo_data['dest_model']}.")
        if args.log_build_errors:
            logging.info(f"Build error: {build_result.stderr}")
    else:
        logging.debug(f"Build succeeded for {repo_data['app']} with model " +
                      f"{repo_data['dest_model']}.")


def build_repo(repo_data, system_config, args, tempdir, ground_truth_build = False):
    ''' Build the repo in the given path with the given config.
    '''
    # Find the target config for this repo per dest model and app name
    target_config = find_config(repo_data["app"], repo_data["dest_model"],
                                args.target_path)

    # Run setup commands if provided
    if "setup_commands" in target_config:
        setup_result = run_setup(repo_data, target_config, args, tempdir)
        if setup_result.returncode != 0:
            logging.error(f"Setup failed for {repo_data['app']} with model " +
                          f"{repo_data['dest_model']}.")
            result = prepare_result_dict(repo_data, setup_result, ground_truth_build)
            result["gen_build_stdout_debug"] = result["build_stdout_debug"]
            result["gen_build_stderr_debug"] = result["build_stderr_debug"]
            return result

    # Get cmds from system, target config
    cmds = list_dep_cmds(system_config, target_config)
    cmds.append(target_config["build_commands_debug"])

    # If ground truth build, replace existing build file
    if ground_truth_build:
        prepare_ground_truth_build(args, tempdir, target_config)

    # Build the repo
    build_result = run_bash(cmds, cwd=tempdir,
                            timeout=target_config["build_timeout"],
                            dry=args.dry,
                            name="build")

    if args.log_build_output:
        logging.info(f"Build output: {build_result.stdout}")

    # Log the build result
    log_build_result(repo_data, build_result, args)

    # Create build result dict to return
    result = prepare_result_dict(repo_data, build_result, ground_truth_build)

    return result
