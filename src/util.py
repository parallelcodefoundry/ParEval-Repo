import logging
import subprocess
import shlex
import os
from subprocess import CompletedProcess

def await_input(prompt, is_valid_input):
    """ Repeatedly ask the user for input until it is valid. """
    response = input(prompt)
    while not is_valid_input(response):
        response = input(prompt)
    return response

def list_dep_commands(system_config, target_config):
    """ Get the dependency commands from the system and target config """
    commands = []
    for dep in target_config["dependencies"]:
        if dep in system_config:
            commands.append(system_config[dep])
        else:
            logging.error(f"Dependency {dep} not found in system config.")
            raise ValueError(f"Dependency {dep} not found in system config.")
    return commands

def run_bash(cmds, cwd=None, timeout=None, dry=False):
    """ Run the given Bash commands on the system and return the result """
    if cwd is None:
        cwd = os.getcwd()
    full_cmd = " && ".join(cmds)
    logging.debug(f"Running command: {full_cmd}")
    logging.debug(f"Running command in directory: {cwd}")
    if dry:
        logging.info(f"Skipping command because --dry was specified: {full_cmd}")
        logging.info(f"Skipped command was in {cwd}")
        return CompletedProcess(args=cmd, returncode=0, stdout="", stderr="")
    else:
        #cmd = shlex.split(cmd)
        return subprocess.run(cmd, capture_output=True, text=True, shell=True, timeout=timeout, cwd=cwd)

def find_config(app, model, target_path):
    """ Find the config for the given app and model """
    logging.debug(f"Looking for config for {app} with model {model} in {target_path}.")
    configs = []
    for root, dirs, files in os.walk(target_path):
        for file in files:
            if file == "config.json":
                with open(os.path.join(root, file), "r") as f:
                    config = json.load(f)
                    configs.append(config)
    logging.debug(f"Found {len(configs)} total configs.")
    for config in configs:
        if config["app"].lower() == app.lower() and config["model"].lower() == model.lower():
            return config
    logging.error(f"No target config found for {app} with model {model}.")
    raise ValueError(f"No target config found for {app} with model {model}.")
