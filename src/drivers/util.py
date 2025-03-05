import logging
import subprocess
import shlex
import os
import json
import shutil
import glob
from subprocess import CompletedProcess
import numpy as np

def await_input(prompt, is_valid_input):
    """ Repeatedly ask the user for input until it is valid. """
    response = input(prompt)
    while not is_valid_input(response):
        response = input(prompt)
    return response

def list_dep_cmds(system_config, target_config):
    """ Get the dependency cmds from the system and target config """
    cmds = [system_config["first"]]
    for dep in target_config["dependencies"]:
        if dep in system_config["dependencies"]:
            cmds.append(system_config["dependencies"][dep])
        else:
            logging.error(f"Dependency {dep} not found in system config.")
            raise ValueError(f"Dependency {dep} not found in system config.")
    return cmds

def run_bash(cmds, cwd=None, timeout=None, dry=False, name=None):
    """ Run the given Bash cmds on the system and return the result """
    if cwd is None:
        cwd = os.getcwd()
    script_name = f"{name}.sh" if name is not None else "temp.sh"
    script_path = os.path.join(cwd, script_name)
    with open(script_path, "w") as f:
        f.write("\n".join(cmds) + "\n")
    os.chmod(script_path, 0o755)
    logging.debug(f"Running commands: {cmds}")
    logging.debug(f"Running commands in directory: {cwd}")
    if dry:
        logging.info(f"Skipping commands because --dry was specified.")
        return CompletedProcess(args=cmds, returncode=0, stdout="", stderr="")
    else:
        full_cmd = shlex.split(f"bash {script_path}")
        try:
            result = subprocess.run(full_cmd, capture_output=True, text=True,
                                    timeout=timeout, cwd=cwd)
            return result
        except subprocess.TimeoutExpired as e:
            logging.debug(f"Timeout occurred: {e}")
            result = CompletedProcess(args=cmds, returncode=124, stdout="", stderr=f"TIMEOUT ({timeout} sec.)")
            return result

def find_config(app, model, target_path):
    """ Find the target config for the given app and model """
    logging.debug(f"Looking for target config for {app} with model {model} in {target_path}.")
    configs = glob.glob(os.path.join(target_path, "**/target.json"), recursive=True)
    configs = [json.load(open(config, "r")) for config in configs]
    logging.debug(f"Found {len(configs)} total target configs.")
    for config in configs:
        if config["app"].lower() == app.lower() and config["model"].lower() == model.lower():
            return config
    logging.error(f"No target config found for {app} with model {model}.")
    raise ValueError(f"No target config found for {app} with model {model}.")

def setup_tempdir(tempdir, code_repo):
    """ Copy the code repo to a temporary directory """
    logging.debug(f"Copying repo to temporary directory: {tempdir}")
    repo_path = os.path.abspath(code_repo['path'])
    shutil.copytree(repo_path, tempdir, dirs_exist_ok=True)

def dict_merge(dct, merge_dct):
    """ Merge elements of merge_dct into dct by key, appending merge_dct values
        to dct values, which are lists. Log an error if merge_dct has a key that
        dct does not have. For any key in dct that is not in merge_dct, add a
        None """
    # Add the merge_dct values to the dct values
    for k, v in merge_dct.items():
        if k in dct:
            dct[k].append(v)
        elif k == "inference_stats":
            continue
        else:
            logging.error(f"Key {k} not found in dictionary.")
            raise ValueError(f"Key {k} not found in dictionary.")

    # Add None to any keys in dct that are not in merge_dct
    for k in dct.keys():
        if k not in merge_dct:
            dct[k].append(None)

def update_results(results, results_row):
    """ Update the results dict of lists with an individual results dictionary,
        matching based on path """
    # Find the row in the results dict that matches the path
    path = results_row["path"]
    if path not in results["path"]:
        logging.error(f"Path {path} not found in results.")
        raise ValueError(f"Path {path} not found in results.")
    row_idx = results["path"].index(path)

    # Update the results dict with the results row
    for key, value in results_row.items():
        if key in results:
            if results[key][row_idx] is None:
                results[key][row_idx] = value
            elif results[key][row_idx] != value:
                logging.error(f"Key already has a non-matching value in results, {results[key][row_idx]} != {value}.")
                raise ValueError(f"Key already has a non-matching value in results, {results[key][row_idx]} != {value}.")
        else:
            logging.error(f"Key {key} not found in results.")
            raise ValueError(f"Key {key} not found in results.")
