""" Class that invokes Codex CLI to perform code translation.
"""
# std imports
import os
import shutil
import subprocess
import json
import time
import atexit
from pathlib import Path
from typing import List, Optional, Dict, Any

# local imports
from translator import Translator
from repo import Repo


class CodexTranslator(Translator):
    """Translator that uses OpenAI Codex CLI to perform code translation."""

    # Constants
    TEMP_REPO_PATH = "/tmp/temp_codex_repo"
    CONTAINER_REPO_PATH = "/temp_codex_repo"
    TRANSLATION_TASK_FILENAME = "translation_task.md"
    EXPERIMENT_METADATA_FILENAME = "experiment_metadata.json"
    SERVE_CHECK_COOLDOWN = 10
    _MAX_SERVE_CHECK_ATTEMPTS = 100
    VLLM_HOST = "127.0.0.1"
    VLLM_PORT = 8000

    # File extensions to remove from output
    REMOVE_EXTENSIONS = (".cu", ".cuh")

    # Git commands
    GIT_INIT = ["git", "init"]
    GIT_ADD_ALL = ["git", "add", "."]
    GIT_COMMIT_INITIAL = ["git", "commit", "-m", "Initial commit"]

    # Instance variables
    _codex_model_name: Optional[str]
    _vllm_environment: Optional[str]
    _vllm_yaml_config: Optional[str]

    _temp_repo_path: str
    _translation_task_path: str
    _output_path: str
    _vllm_launched_from_python: bool

    def __init__(
        self,
        input_repo: Repo,
        output_repos: List[os.PathLike],
        src_model: str,
        dst_model: str,
        dst_config: Dict[str, Any],
        log_interactions: bool = False,
        dry: bool = False,
        hide_progress: bool = False,
        codex_model_name: Optional[str] = None,
        codex_vllm_environment: Optional[str] = None,
        codex_vllm_yaml_config: Optional[str] = None,
    ) -> None:
        super().__init__(
            input_repo,
            output_repos,
            src_model,
            dst_model,
            dst_config,
            log_interactions=log_interactions,
            dry=dry,
            hide_progress=hide_progress,
        )

        self._codex_model_name = codex_model_name
        self._vllm_environment = codex_vllm_environment
        self._vllm_yaml_config = codex_vllm_yaml_config
        self._vllm_launched_from_python = False

        self._temp_repo_path = self.TEMP_REPO_PATH
        self._translation_task_path = os.path.join(
            self._input_repo.path, self.TRANSLATION_TASK_FILENAME
        )
        self._output_path = os.path.join(self._output_paths[0], "repo")

        if self._vllm_environment:
            self._launch_vllm_server(self._vllm_environment, self._vllm_yaml_config)
            self._vllm_launched_from_python = True
        else:
            print("Warning: --codex-vllm-environment not provided; assuming external vLLM server is running.")

    @staticmethod
    def add_args(parser: Any) -> None:
        """Add command line arguments for Codex configuration."""
        parser.add_argument("--codex-model-name", type=str,
                            help="Model name to pass to Codex (e.g. 'openai/gpt-oss-120b').")
        parser.add_argument("--codex-vllm-environment", type=str,
                            help="Path to the Python environment that has vLLM installed (e.g. ~/pssg-venv).")
        parser.add_argument("--codex-vllm-yaml-config", type=str,
                            help="Path to vLLM YAML config file to pass via --config.")

    @staticmethod
    def parse_args(args: Any) -> Dict[str, Any]:
        """Parse command line arguments for Codex configuration."""
        return {
            "codex_model_name": args.codex_model_name,
            "codex_vllm_environment": args.codex_vllm_environment,
            "codex_vllm_yaml_config": args.codex_vllm_yaml_config,
        }

    def translate(self) -> None:
        """Execute the complete translation process using Codex CLI.

        The process includes:
        1. Generate translation task
        2. Initialize temporary repository
        3. Run Codex and capture in-place file changes
        4. Save translated output
        5. Clean up and write metadata
        """
        try:
            self._execute_translation_workflow()
        finally:
            self.cleanup_temp_repo()

    def _execute_translation_workflow(self) -> None:
        """Execute the main translation workflow steps."""
        self.generate_translation_task()
        self.initialize_temp_repo()

        if self.run_codex():
            self._fix_makefile_tabs_and_duplicates()
            print("Saving translated output...")
            self.save_output(self._output_path)
            self.remove_unnecessary_output_files()
            self.write_experiment_metadata()
            self._save_codex_log()
        else:
            print("Translation failed.")

    def generate_translation_task(self) -> None:
        """Generate the translation task file for Codex."""
        print("Generating translation task...")

        translation_task = self._create_translation_task_content()

        try:
            with open(self._translation_task_path, "w", encoding="utf-8") as f:
                f.write(translation_task)
            print(f"Translation task generated: {self._translation_task_path}")
        except IOError as e:
            print(f"Error writing translation task: {e}")
            raise

    def _create_translation_task_content(self) -> str:
        """Create the content for the translation task file."""
        data = self._dst_config

        prompt = (
            f"You are a helpful coding assistant. You are helping a software developer translate a "
            f"codebase from the {self._src_model} execution model to the {self._dst_model} execution "
            f"model.\n\n"
            f"The codebase is called {data['app']}. Its path is {self.TEMP_REPO_PATH}. Given this code "
            f"repository, translate the {data['app']} codebase's {self._src_model}-specific files to "
            f"the {self._dst_model} execution model.\n\n"
            f"The new files should be in {data['filename_desc']} and all old {self._src_model} files "
            f"must be deleted. You may use standard command-line tools (e.g., the `rm` command) to "
            f"remove obsolete {self._src_model}-specific files. A new {data['build_filename']} should "
            f"be made to compile accordingly with the new files.\n\n"
            f"Ensure that the user can compile this code using, for example, `{data['ex_build_cmd']}` "
            f"to build the code for {data['ex_build_desc']}. Ensure also that the command line "
            f"interface after translation still works as expected, so that, for example, "
            f"`{data['ex_run_cmd']}` still works to run the code with {data['ex_run_desc']}."
        )
        return prompt.strip()

    def initialize_temp_repo(self) -> None:
        """Initialize the temporary repository and perform initial Git setup."""
        print("Initializing temporary Git repository...")
        self._prepare_temp_directory()
        self._copy_source_to_temp()
        self._initialize_git_repo()

    def _prepare_temp_directory(self) -> None:
        """Remove existing temp directory if it exists."""
        if os.path.exists(self._temp_repo_path):
            print("The temporary repository exists. Removing the repository...")
            shutil.rmtree(self._temp_repo_path)

    def _copy_source_to_temp(self) -> None:
        """Copy the original repository to the temporary directory."""
        shutil.copytree(self._input_repo.path, self._temp_repo_path, dirs_exist_ok=True)

    def _initialize_git_repo(self) -> None:
        """Initialize Git repository and make initial commit."""
        subprocess.run(self.GIT_INIT, cwd=self._temp_repo_path, check=True)
        subprocess.run(self.GIT_ADD_ALL, cwd=self._temp_repo_path, check=True)
        subprocess.run(self.GIT_COMMIT_INITIAL, cwd=self._temp_repo_path, check=True)

    def run_codex(self) -> bool:
        """Run the Codex CLI command. Codex modifies files in-place."""
        try:
            with open(self._translation_task_path, "r", encoding="utf-8") as f:
                prompt = f.read()
        except IOError as e:
            print(f"Error reading translation task: {e}")
            return False

        command = self._build_codex_command(prompt)
        env = self._build_codex_env()
        print(f"Running Codex command: {' '.join(command[:4])} ...")

        try:
            proc = subprocess.Popen(
                command, text=True, cwd=self._temp_repo_path, env=env,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            )
            log_lines = []
            for line in proc.stdout:
                print(line, end="", flush=True)
                log_lines.append(line)
            proc.wait()
            self._codex_log = "".join(log_lines)
            if proc.returncode != 0:
                print(f"Codex exited with return code {proc.returncode}")
                return False
            print("Codex command executed successfully.")
            return True
        except Exception as e:
            self._codex_log = "".join(log_lines) if 'log_lines' in dir() else ""
            print(f"An error occurred running Codex: {e}")
            return False

    def _build_codex_command(self, prompt: str) -> List[str]:
        """Build the Codex CLI command with all required parameters."""
        cmd = ["codex", "exec", "--dangerously-bypass-approvals-and-sandbox"]
        if self._codex_model_name:
            cmd.extend(["--model", self._codex_model_name])
        cmd.append(prompt)
        return cmd

    def _build_codex_env(self) -> dict:
        """Build the subprocess environment for the Codex command."""
        env = os.environ.copy()
        if self._vllm_launched_from_python:
            base_url = f"http://{self.VLLM_HOST}:{self.VLLM_PORT}/v1"
            # Set both names: Node.js SDK (Codex) reads OPENAI_BASE_URL;
            # some tools also check OPENAI_API_BASE.
            env["OPENAI_BASE_URL"] = base_url
            env["OPENAI_API_BASE"] = base_url
        if "OPENAI_API_KEY" not in env:
            env["OPENAI_API_KEY"] = "dummy-local-ok"
        # Prevent interactive pagers from blocking Codex when it runs
        # git log, man, or other pager-triggering commands internally.
        env.setdefault("PAGER", "cat")
        env.setdefault("MANPAGER", "cat")
        env.setdefault("GIT_PAGER", "cat")
        env.setdefault("LESS", "-R")
        return env

    def save_output(self, output_dir: str) -> None:
        """Copy the contents of the temporary repository to the final output directory.

        Removes the .git directory to prepare for adding to the results repository.
        """
        try:
            if os.path.exists(output_dir):
                shutil.rmtree(output_dir)
            shutil.copytree(self._temp_repo_path, output_dir, dirs_exist_ok=True)

            # Remove .git directory
            git_dir = os.path.join(output_dir, ".git")
            if os.path.exists(git_dir):
                shutil.rmtree(git_dir)
        except (OSError, shutil.Error) as e:
            print(f"Error saving output: {e}")
            raise

    def remove_unnecessary_output_files(self) -> None:
        """Remove unnecessary files (any .cu or .cuh files) from the output."""
        print(f"Cleaning the output repository: {self._output_path}")

        try:
            self._remove_files_by_extension(self._output_path, self.REMOVE_EXTENSIONS)
            print(f"Finished cleaning the output repository: {self._output_path}")
        except OSError as e:
            print(f"Error cleaning output files: {e}")
            raise

    def _remove_files_by_extension(self, directory: str, extensions: tuple) -> None:
        """Remove files with specified extensions from a directory tree."""
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith(extensions):
                    file_path = os.path.join(root, file)
                    os.remove(file_path)

    def _fix_makefile_tabs_and_duplicates(self) -> None:
        makefile = Path(self._temp_repo_path) / "Makefile"
        if not makefile.exists():
            return

        lines = makefile.read_text(encoding="utf-8", errors="replace").splitlines(True)

        # 1) Remove exact duplicate lines (preserve order)
        print("Removing duplicate lines in the Makefile...")
        seen = set()
        duplicates = []
        for line in lines:
            if line not in seen:
                seen.add(line)
                duplicates.append(line)
        lines = duplicates

        # 2) Enforce Makefile tab rules
        print("Fixing Makefile tabs...")
        i = 0
        while i < len(lines) - 1:
            curr = lines[i].lstrip()
            nxt = lines[i + 1]

            is_rule = ":" in curr
            is_conditional = curr.startswith((
                "ifeq",
                "ifneq",
                "ifdef",
                "ifndef",
                "else"
            ))

            if is_rule or is_conditional:
                if nxt.strip() and not nxt.startswith("\t") and not nxt.lstrip().startswith("#"):
                    lines[i + 1] = "\t" + nxt
            i += 1

        makefile.write_text("".join(lines), encoding="utf-8")

    def write_experiment_metadata(self) -> None:
        """Write experiment metadata to a JSON file in the output directory."""
        exp_meta_fpath = os.path.join(self._output_path, "..", self.EXPERIMENT_METADATA_FILENAME)

        try:
            os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)

            metadata = self._create_experiment_metadata()

            with open(exp_meta_fpath, "w", encoding="utf-8") as f:
                json.dump(metadata, f, indent=4)

            print(f"Experiment metadata written to {exp_meta_fpath}.")
        except (OSError, json.JSONEncodeError) as e:
            print(f"Error writing experiment metadata: {e}")
            raise

    def _create_experiment_metadata(self) -> Dict[str, Any]:
        """Create the experiment metadata dictionary."""
        output_number = int(self._output_path.split("/")[-2][7:])

        return {
            "app": self._dst_config["app"],
            "prompt_strategy": "Codex",
            "llm_name": self._codex_model_name,
            "source_model": self._src_model,
            "dest_model": self._dst_model,
            "output_number": output_number,
            "path": self._output_path,
        }

    def _save_codex_log(self) -> None:
        """Save the captured Codex CLI output to a log file in the output directory."""
        log_path = os.path.join(self._output_path, "..", "codex_log.txt")
        try:
            with open(log_path, "w", encoding="utf-8") as f:
                f.write(getattr(self, "_codex_log", ""))
            print(f"Codex log saved to {log_path}")
        except OSError as e:
            print(f"Error saving Codex log: {e}")

    def cleanup_temp_repo(self) -> None:
        """Remove the temporary repository."""
        print("Cleaning up temporary repository...")

        try:
            if os.path.exists(self._temp_repo_path):
                shutil.rmtree(self._temp_repo_path)
            print("Temporary repository cleaned up.")
        except OSError as e:
            print(f"Error cleaning up temporary repository: {e}")
            # Don't raise here as this is cleanup code

    def _launch_vllm_server(self, environment_path: str, yaml_config: Optional[str] = None):
        """Launch a vLLM server in the background using the Python environment directory
           provided.
        """
        # Early exit if vLLM server is already running
        if subprocess.run(
            ["curl", f"http://{self.VLLM_HOST}:{self.VLLM_PORT}/health"],
            capture_output=True, text=True, check=False
        ).returncode == 0:
            return None
        py_executable = os.path.join(environment_path, "bin", "python")
        vllm_command = [
            py_executable, "-m", "vllm.entrypoints.openai.api_server",
            "--host", self.VLLM_HOST,
            "--port", str(self.VLLM_PORT),
        ]
        vllm_api_key = os.getenv("VLLM_API_KEY")
        if self._codex_model_name is not None:
            vllm_command.extend(["--model", self._codex_model_name])
        if vllm_api_key is not None:
            vllm_command.extend(["--api-key", vllm_api_key])
        if yaml_config:
            vllm_command.extend(["--config", yaml_config])
        print("Full vLLM subprocess command:", " ".join(vllm_command))
        vllm_server = subprocess.Popen(vllm_command)
        # Ping the server until it is ready at the health endpoint
        checking, num_attempts = True, 0
        while checking and num_attempts < self._MAX_SERVE_CHECK_ATTEMPTS:
            status = subprocess.run(
                ["curl", f"http://{self.VLLM_HOST}:{self.VLLM_PORT}/health"],
                capture_output=True, text=True, check=False
            )
            if status.returncode == 0:
                checking = False
            else:
                print(f"vLLM server not ready, checking again after {self.SERVE_CHECK_COOLDOWN} seconds...")
                time.sleep(self.SERVE_CHECK_COOLDOWN)
                num_attempts += 1
        atexit.register(vllm_server.terminate)
        print("vLLM server ready.")
        return vllm_server
