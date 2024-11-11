import logging
import subprocess
import shlex
import os
import json
import sys
import shutil
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
    cmds = []
    for dep in target_config["dependencies"]:
        if dep in system_config:
            cmds.append(system_config[dep])
        else:
            logging.error(f"Dependency {dep} not found in system config.")
            raise ValueError(f"Dependency {dep} not found in system config.")
    return cmds

def run_bash(cmds, cwd=None, timeout=None, dry=False):
    """ Run the given Bash cmds on the system and return the result """
    if cwd is None:
        cwd = os.getcwd()
    script_name = sys._getframe(1).f_code.co_name + "_script.sh"
    script_path = os.path.join(cwd, script_name)
    with open(script_path, "w") as f:
        f.write("\n".join(cmds))
    os.chmod(script_path, 0o755)
    logging.debug(f"Running commands: {cmds}")
    logging.debug(f"Running commands in directory: {cwd}")
    if dry:
        logging.info(f"Skipping commands because --dry was specified.")
        return CompletedProcess(args=cmds, returncode=0, stdout="", stderr="")
    else:
        full_cmd = shlex.split(f"bash {script_path}")
        return subprocess.run(full_cmd, capture_output=True, text=True,
                              timeout=timeout, cwd=cwd)

def find_config(app, model, target_path):
    """ Find the target config for the given app and model """
    logging.debug(f"Looking for target config for {app} with model {model} in {target_path}.")
    configs = []
    for root, dirs, files in os.walk(target_path):
        for file in files:
            if file == "target.json":
                with open(os.path.join(root, file), "r") as f:
                    config = json.load(f)
                    configs.append(config)
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

def meta_to_arr(meta):
    """ Convert a metadata dictionary to an array for the results DataFrame. """
    return np.array([meta["app"],
                     meta["prompt_strategy"],
                     meta["llm_name"],
                     meta["source_model"],
                     meta["dest_model"],
                     meta["output_number"],
                     meta["path"],
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None])

def update_results(results, results_dict):
    """ Update the results DataFrame with a results dictionary. """
    for key, value in results_dict.items():
        if (key in results.columns
            and results.loc[results["path"] == results_dict["path"], key].isnull().any()):
            results.loc[results["path"] == results_dict["path"], key] = str(value) if isinstance(value, list) else value
