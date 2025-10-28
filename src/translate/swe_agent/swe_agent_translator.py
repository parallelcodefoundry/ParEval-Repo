""" Class that invokes SWE-agent to perform code translation.
"""
# std imports
import os
import shutil
import subprocess
import json
import time
from typing import List, Optional, Dict, Any

# local imports
from translator import Translator
from repo import Repo

class SWEAgentTranslator(Translator):
    """Translator that uses SWE-agent to perform code translation."""

    # Constants
    TEMP_REPO_PATH = "/tmp/temp_sweagent_repo"
    TRANSLATION_TASK_FILENAME = "translation_task.md"
    TRAJECTORIES_DIR = "trajectories"
    PATCH_FILENAME = "temp.patch"
    EXPERIMENT_METADATA_FILENAME = "experiment_metadata.json"
    SERVE_CHECK_COOLDOWN = 10

    # File extensions to remove from output
    REMOVE_EXTENSIONS = (".cu", ".cuh")

    # Git commands
    GIT_INIT = ["git", "init"]
    GIT_ADD_ALL = ["git", "add", "."]
    GIT_COMMIT_INITIAL = ["git", "commit", "-m", "Initial commit"]

    # Instance variables
    _swe_agent_model_name: str
    _swe_agent_per_instance_cost_limit: float
    _swe_agent_config: Optional[str]
    _swe_agent_parser: Optional[str]
    _swe_agent_max_input_token: Optional[int]

    _temp_repo_path: str
    _translation_task_path: str
    _output_path: str

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
        swe_agent_model_name: Optional[str] = None,
        swe_agent_per_instance_cost_limit: float = 0.06,
        swe_agent_config: Optional[str] = None,
        swe_agent_parser: Optional[str] = None,
        swe_agent_max_input_token: Optional[int] = None,
    ) -> None:
        super().__init__(
            input_repo,
            output_repos,
            src_model,
            dst_model,
            dst_config,
            log_interactions=log_interactions,
            dry=dry,
            hide_progress=hide_progress
        )

        self._swe_agent_model_name = swe_agent_model_name
        self._swe_agent_per_instance_cost_limit = swe_agent_per_instance_cost_limit
        self._swe_agent_config = swe_agent_config
        self._swe_agent_parser = swe_agent_parser
        self._swe_agent_max_input_token = swe_agent_max_input_token

        self._temp_repo_path = self.TEMP_REPO_PATH
        self._translation_task_path = os.path.join(
            self._input_repo.path, self.TRANSLATION_TASK_FILENAME
        )
        self._output_path = os.path.join(self._output_paths[0], "repo")

        if self._is_ollama_model(self._swe_agent_model_name):
            if self._swe_agent_parser is None:
                self._swe_agent_parser = "thought_action"
            if self._swe_agent_max_input_token is None:
                self._swe_agent_max_input_token = 4096
            self._launch_ollama_server()

    @staticmethod
    def _is_ollama_model(name: str) -> bool:
        name = (name or "").lower()
        return name.startswith("ollama/")


    def _launch_ollama_server(self) -> None:
        """Launch an Ollama server in the background."""
        # Check that ollama is installed
        if not shutil.which("ollama"):
            raise ValueError("Ollama is not in the path. Please install Ollama and add it to the path.")
        # Early exit if ollama is already running
        if subprocess.run(["ollama", "list"], capture_output=True, text=True).returncode == 0:
            return
        ollama_command = ["ollama", "serve"]
        subprocess.Popen(ollama_command, stdout=subprocess.DEVNULL,
        stderr=subprocess.STDOUT,       
        stdin=subprocess.DEVNULL,   
        start_new_session=True,
        env=env)
        # Check that the server is running
        checking = True
        while checking:
            status = subprocess.run(["ollama", "list"], capture_output=True, text=True)
            if status.returncode == 0:
                checking = False
            else:
                print(f"Ollama server not ready, checking again after {self.SERVE_CHECK_COOLDOWN} seconds...")
                time.sleep(self.SERVE_CHECK_COOLDOWN)
        print(f"Ollama server ready.")
        return


    @staticmethod
    def add_args(parser: Any) -> None:
        """Add command line arguments for SWE-agent configuration."""
        parser.add_argument("--swe-agent-model-name", type=str,
                            help="Name of the agent model to use (e.g. 'gpt-4o', 'ollama/llama3.2:latest').")
        parser.add_argument("--swe-agent-per-instance-cost-limit", type=float,
                            help="Per-instance cost limit for the agent model; set to 0 for local models.")
        parser.add_argument("--swe-agent-config", type=str,
                            help="Custom config file provided to SWE-Agent. Default config file is used if none is provided.")
        parser.add_argument("--swe-agent-parser", type=str, choices=["thought_action", "function_call"],
                            help="Parsing strategy. Use 'thought_action' for local/Ollama models.")
        parser.add_argument("--swe-agent-max-input-token", type=int,
                            help="Override max input tokens to avoid local-model warnings.")

    @staticmethod
    def parse_args(args: Any) -> Dict[str, Any]:
        """Parse command line arguments for SWE-agent configuration."""
        return {
            "swe_agent_model_name": args.swe_agent_model_name,
            "swe_agent_per_instance_cost_limit": args.swe_agent_per_instance_cost_limit,
            "swe_agent_config": args.swe_agent_config,
            "swe_agent_parser": args.swe_agent_parser,
            "swe_agent_max_input_token": args.swe_agent_max_input_token,
        }

    def translate(self) -> None:
        """Execute the complete translation process using SWE-agent.

        The process includes:
        1. Generate translation task
        2. Initialize temporary repository
        3. Run SWE-agent and apply patches
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

        if self.run_swe_agent():
            print("Saving translated output...")
            self.save_output(self._output_path)
            self.remove_unnecessary_output_files()
            self.write_experiment_metadata()
        else:
            print("Translation failed.")


    def generate_translation_task(self) -> None:
        """Generate the translation task file for SWE-agent."""
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
            f"The codebase is called {data['app']}. Its path is {data['path']}. Given this code "
            f"repository, translate the {data['app']} codebase's {self._src_model}-specific files to "
            f"the {self._dst_model} execution model.\n\n"
            f"The new files should be in {data['filename_desc']} and all old {self._src_model} files "
            f"must be deleted. A new {data['build_filename']} should be made to compile accordingly "
            f"with the new files.\n\n"
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

    def run_swe_agent(self) -> bool:
        """Run the SWE-agent command and apply the resulting patch."""
        command = self._build_swe_agent_command()
        print(f"Running SWE-agent command: {' '.join(command)}")

        try:
            process = subprocess.run(command, text=True, cwd=self._temp_repo_path, check=True)

            if process.returncode == 0:
                print("SWE-agent command executed successfully.")
                return self._apply_swe_agent_patch()
            else:
                print(f"Command failed with return code {process.returncode}.")
                return False

        except Exception as e:
            print(f"An error occurred: {e}")
            return False

    def _build_swe_agent_command(self) -> List[str]:
        """Build the SWE-agent command with all required parameters."""
        cmd = [
            "sweagent", "run",
            f"--agent.model.name={self._swe_agent_model_name}",
            f"--agent.model.per_instance_cost_limit={self._swe_agent_per_instance_cost_limit}",
            f"--env.repo.path={self._temp_repo_path}",
            f"--problem_statement.path={self._translation_task_path}",
        ]

        if self._swe_agent_parser:
            cmd.append(f"--agent.tools.parse_function.type={self._swe_agent_parser}")
        if self._swe_agent_max_input_token:
            cmd.append(f"--agent.model.max_input_tokens={self._swe_agent_max_input_token}")
        if self._swe_agent_config:
            cmd.extend(["--config", self._swe_agent_config])

        return cmd

    def _apply_swe_agent_patch(self) -> bool:
        """Find and apply the patch file generated by SWE-agent."""
        print("Applying patch...")

        trajectories_dir = os.path.join(self._temp_repo_path, self.TRAJECTORIES_DIR)
        patch_file_path = self._find_patch_file(trajectories_dir)

        if patch_file_path is None:
            print("Error: No patch file found in trajectories directory.")
            return False

        return self._apply_patch_file(patch_file_path)

    def _find_patch_file(self, trajectories_dir: str) -> Optional[str]:
        """Find the patch file in the trajectories directory."""
        for root, _, files in os.walk(trajectories_dir):
            for file in files:
                if file.endswith(".patch"):
                    return os.path.join(root, file)
        return None

    def _apply_patch_file(self, patch_file_path: str) -> bool:
        """Rename and apply the patch file to the repository."""
        new_patch_path = os.path.join(self._temp_repo_path, self.PATCH_FILENAME)

        try:
            os.rename(patch_file_path, new_patch_path)
            subprocess.run(
                ["patch", "-p1", "-i", new_patch_path],
                cwd=self._temp_repo_path,
                check=True
            )
            print("Patch applied successfully.")
            return True
        except (OSError, subprocess.CalledProcessError) as e:
            print(f"Error applying patch: {e}")
            return False


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


    def write_experiment_metadata(self) -> None:
        """Write experiment metadata to a JSON file in the output directory."""
        exp_meta_fpath = os.path.join(self._output_path, "..", self.EXPERIMENT_METADATA_FILENAME)

        try:
            os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)

            metadata = self._create_experiment_metadata()

            with open(exp_meta_fpath, 'w', encoding='utf-8') as f:
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
            "prompt_strategy": "SWE-agent",
            "llm_name": self._swe_agent_model_name,
            "source_model": self._src_model,
            "dest_model": self._dst_model,
            "output_number": output_number,
            "path": self._output_path
        }


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
