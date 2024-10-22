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

def run_bash(cmd, cwd=None, timeout=None, dry=False):
    """ Run the given Bash command on the system and return the result """
    if cwd is None:
        cwd = os.getcwd()
    logging.debug(f"Running command: {cmd}")
    logging.debug(f"Running command in directory: {cwd}")
    if dry:
        logging.info(f"Skipping command because --dry was specified: {cmd}")
        logging.info(f"Skipped command was in {cwd}")
        return CompletedProcess(args=cmd, returncode=0, stdout="", stderr="")
    else:
        cmd = shlex.split(cmd)
        return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)

def find_config(configs, app, model):
    """ Find the config for the given app and model """
    logging.debug(f"Looking for config for {app} with model {model}.")
    for config in configs:
        if config["app"] == app.lower() and config["model"] == model.lower():
            return config
    logging.error(f"No config found for {app} with model {model}.")
    return None
