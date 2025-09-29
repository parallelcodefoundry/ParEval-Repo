#!/usr/bin/env python3
"""
Utility functions for the ParEval driver scripts.
This module contains core utility functions for configuration, execution, data management,
and common operations to reduce code duplication.
"""
import logging
import subprocess
import shlex
import os
import json
import shutil
import glob
from subprocess import CompletedProcess
from typing import Dict, List, Any, Callable


class InputHandler:
    """Handles user input validation and prompting."""

    @staticmethod
    def await_input(prompt: str, is_valid_input: Callable[[str], bool]) -> str:
        """Repeatedly ask the user for input until it is valid."""
        response = input(prompt)
        while not is_valid_input(response):
            response = input(prompt)
        return response


class CommandExecutor:
    """Handles command execution and dependency management."""

    @staticmethod
    def list_dep_cmds(system_config: Dict[str, Any],
                     target_config: Dict[str, Any]) -> List[str]:
        """Get the dependency cmds from the system and target config."""
        cmds = []
        for dep in target_config["dependencies"]:
            if dep in system_config:
                cmds.append(system_config[dep])
            else:
                logging.error(f"Dependency {dep} not found in system config.")
                raise ValueError(f"Dependency {dep} not found in system config.")
        return cmds

    @staticmethod
    def run_bash(cmds: List[str], cwd: str = None, timeout: int = None,
                dry: bool = False, name: str = None) -> CompletedProcess:
        """Run the given Bash cmds on the system and return the result."""
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
            logging.info("Skipping commands because --dry was specified.")
            return CompletedProcess(args=cmds, returncode=0, stdout="", stderr="")

        full_cmd = shlex.split(f"bash {script_path}")
        try:
            result = subprocess.run(full_cmd, capture_output=True, text=True,
                                    timeout=timeout, cwd=cwd)
            return result
        except subprocess.TimeoutExpired as e:
            logging.debug(f"Timeout occurred: {e}")
            return CompletedProcess(args=cmds, returncode=124,
                                  stdout="", stderr=f"TIMEOUT ({timeout} sec.)")


class ConfigManager:
    """Manages configuration file operations."""

    @staticmethod
    def find_config(app: str, model: str, target_path: str) -> Dict[str, Any]:
        """Find the target config for the given app and model."""
        logging.debug(f"Looking for target config for {app} with model {model} in {target_path}.")

        configs = glob.glob(os.path.join(target_path, "**/target.json"), recursive=True)
        configs = [json.load(open(config, "r")) for config in configs]

        logging.debug(f"Found {len(configs)} total target configs.")

        for config in configs:
            if (config["app"].lower() == app.lower() and
                config["model"].lower() == model.lower()):
                return config

        logging.error(f"No target config found for {app} with model {model}.")
        raise ValueError(f"No target config found for {app} with model {model}.")


class DataManager:
    """Manages data operations and dictionary manipulations."""

    @staticmethod
    def dict_merge(dct: Dict[str, List], merge_dct: Dict[str, Any]) -> None:
        """Merge elements of merge_dct into dct by key, appending merge_dct values
        to dct values, which are lists. Log an error if merge_dct has a key that
        dct does not have. For any key in dct that is not in merge_dct, add a None.
        """
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

    @staticmethod
    def empty_other_keys_at_idx(dct: Dict[str, List], merge_dct: Dict[str, Any],
                               idx: int) -> None:
        """Set keys other than those in merge_dct to None at idx in dct."""
        for k in dct.keys():
            if k not in merge_dct:
                if len(dct[k]) <= idx:
                    raise ValueError(f"Key {k} in dct has length {len(dct[k])} but trying to write at index {idx}.")
                dct[k][idx] = None

    @staticmethod
    def empty_results_dict() -> Dict[str, List]:
        """Create an empty results dictionary."""
        return {
            "app": [],
            "prompt_strategy": [],
            "llm_name": [],
            "source_model": [],
            "dest_model": [],
            "output_number": [],
            "path": [],
            "ground_truth_build": [],
            "build_result_debug": [],
            "build_stdout_debug": [],
            "build_stderr_debug": [],
            "gen_build_stdout_debug": [],
            "gen_build_stderr_debug": [],
            "run_results_debug": [],
            "run_exec_checks_debug": [],
            "run_stdouts_debug": [],
            "run_stderrs_debug": [],
            "tempdir_path": []
        }


class ResultBuilder:
    """Builder class for creating result dictionaries with consistent structure."""

    @staticmethod
    def build_result(repo_data: Dict[str, str],
                    ground_truth_build: bool = False,
                    **kwargs) -> Dict[str, Any]:
        """Create a base result dictionary with common fields."""
        result = {
            "path": repo_data["path"],
            "ground_truth_build": ground_truth_build
        }
        result.update(kwargs)
        return result

    @staticmethod
    def build_build_result(repo_data: Dict[str, str],
                          run_result: CompletedProcess,
                          ground_truth_build: bool = False) -> Dict[str, Any]:
        """Create a build result dictionary."""
        return ResultBuilder.build_result(
            repo_data, ground_truth_build,
            build_result_debug=run_result.returncode,
            build_stdout_debug=run_result.stdout,
            build_stderr_debug=run_result.stderr
        )

    @staticmethod
    def build_run_result(repo_data: Dict[str, str],
                        run_results: List[int],
                        run_exec_checks: List[int],
                        run_stdouts: List[str],
                        run_stderrs: List[str]) -> Dict[str, Any]:
        """Create a run result dictionary."""
        return ResultBuilder.build_result(
            repo_data,
            run_results_debug=run_results,
            run_exec_checks_debug=run_exec_checks,
            run_stdouts_debug=run_stdouts,
            run_stderrs_debug=run_stderrs
        )

    @staticmethod
    def build_skip_run_result(repo_data: Dict[str, str]) -> Dict[str, Any]:
        """Create a result dictionary for a skipped run."""
        return ResultBuilder.build_result(
            repo_data,
            run_results_debug=[1],
            run_exec_checks_debug=[0],
            run_stdouts_debug=[""],
            run_stderrs_debug=["skip due to build failure"]
        )


class LoggingHelper:
    """Helper class for consistent logging patterns."""

    @staticmethod
    def log_build_result(repo_data: Dict[str, str],
                        build_result: CompletedProcess,
                        args: Any) -> None:
        """Log build results consistently."""
        app = repo_data['app']
        model = repo_data['dest_model']

        if build_result.returncode != 0:
            logging.debug(f"Build failed for {app} with model {model}.")
            if args.log_build_errors:
                logging.info(f"Build error: {build_result.stderr}")
        else:
            logging.debug(f"Build succeeded for {app} with model {model}.")

    @staticmethod
    def log_run_result(repo_data: Dict[str, str],
                      run_result: CompletedProcess,
                      args: Any,
                      test_index: int) -> None:
        """Log run results consistently."""
        app = repo_data['app']
        model = repo_data['dest_model']

        if run_result.returncode != 0:
            logging.debug(f"Run failed for {app} with model {model} test {test_index}.")
            if args.log_run_errors:
                logging.info(f"Run error: {run_result.stderr}")
        else:
            logging.debug(f"Run succeeded for {app} with model {model} test {test_index}.")


class FileSystemHelper:
    """Helper class for file system operations."""

    @staticmethod
    def setup_tempdir(tempdir: str, code_repo: Dict[str, str]) -> None:
        """Copy the code repo to a temporary directory."""
        logging.debug(f"Copying repo to temporary directory: {tempdir}")
        repo_path = os.path.abspath(code_repo['path'])
        shutil.copytree(repo_path, tempdir, dirs_exist_ok=True)

    @staticmethod
    def save_temps(tempdir: str, save_temps_dir: str,
                  results_row: Dict[str, Any]) -> None:
        """Save temporary files to the provided directory."""
        tempdir_name = os.path.basename(tempdir)
        tempdir_path = os.path.join(save_temps_dir, tempdir_name)
        logging.info(f"Saving temporary directory to {tempdir_path}.")
        shutil.copytree(tempdir, tempdir_path)
        results_row["tempdir_path"] = str(tempdir_path)

    @staticmethod
    def cleanup_tempdir(tempdir: str) -> None:
        """Clean up temporary directory subdirectories."""
        # Remove all subdirs in tempdir manually before exiting when block,
        # workaround for oserror race condition bug
        for root, dirs, _ in os.walk(tempdir):
            for d in dirs:
                shutil.rmtree(os.path.join(root, d), ignore_errors=True)


class ConfigHelper:
    """Helper class for configuration operations."""

    @staticmethod
    def prepare_ground_truth_build(args: Any, tempdir: str,
                                  target_config: Dict[str, Any]) -> None:
        """Rewrite the existing build file with the ground truth build file."""
        build_fname = target_config["build_filename"]
        gen_build_fname = os.path.join(tempdir, build_fname)
        true_build_fname = os.path.join(
            os.path.dirname(os.path.dirname(args.target_path)),
            target_config["path"],
            build_fname
        )
        logging.debug(f"Rewriting build file {gen_build_fname} with ground "
                     f"truth build file {true_build_fname}.")
        shutil.copyfile(true_build_fname, gen_build_fname)


class ValidationHelper:
    """Helper class for validation operations."""

    @staticmethod
    def check_output(repo_data: Dict[str, str],
                    target_config: Dict[str, Any],
                    run_result: CompletedProcess,
                    test_index: int) -> int:
        """Check the run output against the expected output."""
        if target_config["debug_type"] == "match":
            expected = target_config["debug_outputs"][test_index]
            if expected not in run_result.stdout:
                logging.debug(f"Output mismatch for {repo_data['app']} with model "
                            f"{repo_data['dest_model']} test {test_index}.")
                logging.debug(f"Expected output to contain: {expected}")
                logging.debug(f"Actual output: {run_result.stdout}")
                return 1
        return 0

    @staticmethod
    def check_exec(repo_data: Dict[str, str],
                  target_config: Dict[str, Any],
                  system_config: Dict[str, Any],
                  test_index: int,
                  args: Any,
                  tempdir: str) -> int:
        """Check that binary executes as expected for this input."""
        if system_config["exec_check"] != "":
            cmds = CommandExecutor.list_dep_cmds(system_config, target_config)
            exc_cmd = [system_config["exec_check"] + " " +
                      target_config["run_commands_perf"][0]]
            exec_check_result = CommandExecutor.run_bash(
                cmds + exc_cmd,
                cwd=tempdir,
                timeout=target_config["run_timeout"],
                dry=args.dry
            )

            fail_text = system_config["exec_check_fail_text"]
            if fail_text == "":
                logging.error(f"exec_check_fail_text not specified in {args.system_config}.")
                raise ValueError(f"exec_check_fail_text not specified in {args.system_config}.")

            if fail_text in exec_check_result.stdout or fail_text in exec_check_result.stderr:
                logging.debug(f"Execution check failed for {repo_data['app']} with model "
                            f"{repo_data['dest_model']} test {test_index}.")
                return 1
        else:
            logging.debug(f"No execution check specified in {args.system_config}.")
        return 0


class ResultsManager:
    """Manager class for handling results operations."""

    @staticmethod
    def save_run_result(run_result: CompletedProcess,
                       exec_check: int,
                       run_results: List[int],
                       run_exec_checks: List[int],
                       run_stdouts: List[str],
                       run_stderrs: List[str]) -> None:
        """Save the run result to the appropriate lists."""
        run_results.append(run_result.returncode)
        run_exec_checks.append(exec_check)
        run_stdouts.append(run_result.stdout)
        run_stderrs.append(run_result.stderr)


# Legacy function aliases for backward compatibility
def await_input(prompt: str, is_valid_input: Callable[[str], bool]) -> str:
    """Legacy function - use InputHandler.await_input instead."""
    return InputHandler.await_input(prompt, is_valid_input)


def list_dep_cmds(system_config: Dict[str, Any], target_config: Dict[str, Any]) -> List[str]:
    """Legacy function - use CommandExecutor.list_dep_cmds instead."""
    return CommandExecutor.list_dep_cmds(system_config, target_config)


def run_bash(cmds: List[str], cwd: str = None, timeout: int = None,
            dry: bool = False, name: str = None) -> CompletedProcess:
    """Legacy function - use CommandExecutor.run_bash instead."""
    return CommandExecutor.run_bash(cmds, cwd, timeout, dry, name)


def find_config(app: str, model: str, target_path: str) -> Dict[str, Any]:
    """Legacy function - use ConfigManager.find_config instead."""
    return ConfigManager.find_config(app, model, target_path)


def setup_tempdir(tempdir: str, code_repo: Dict[str, str]) -> None:
    """Copy the code repo to a temporary directory."""
    FileSystemHelper.setup_tempdir(tempdir, code_repo)


def dict_merge(dct: Dict[str, List], merge_dct: Dict[str, Any]) -> None:
    """Legacy function - use DataManager.dict_merge instead."""
    DataManager.dict_merge(dct, merge_dct)


def empty_other_keys_at_idx(dct: Dict[str, List], merge_dct: Dict[str, Any], idx: int) -> None:
    """Legacy function - use DataManager.empty_other_keys_at_idx instead."""
    DataManager.empty_other_keys_at_idx(dct, merge_dct, idx)


def empty_results_dict() -> Dict[str, List]:
    """Legacy function - use DataManager.empty_results_dict instead."""
    return DataManager.empty_results_dict()
