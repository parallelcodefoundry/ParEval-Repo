""" Class that invokes SWE-agent to perform code translation.
"""
# std imports
import os
import shutil
import subprocess
import json
from typing import List

# local imports
from translator import Translator
from repo import Repo

class SWEAgentTranslator(Translator):

    _swe_agent_model_name: str
    _swe_agent_per_instance_cost_limit: float
    _temp_repo_path: str
    _translation_task_path: str
    _output_path: str

    def __init__(
        self,
        input_repo: Repo,
        output_repos: List[os.PathLike],
        src_model: str,
        dst_model: str,
        dst_config: dict,
        log_interactions=False,
        dry=False,
        hide_progress=False,
        swe_agent_model_name=None,
        swe_agent_per_instance_cost_limit=0.06
    ):
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
        self._temp_repo_path = "/tmp/temp_sweagent_repo"
        self._translation_task_path = os.path.join(self._input_repo.path, "translation_task.md")
        self._output_path = os.path.join(self._output_paths[0], "repo")


    @staticmethod
    def add_args(parser):
        parser.add_argument("--swe-agent-model-name", type=str,
                            help="Name of the agent model to use (e.g. 'gpt-4o').")
        parser.add_argument("--swe-agent-per-instance-cost-limit", type=float,
                            help="Per-instance cost limit for the agent model.")


    @staticmethod
    def parse_args(args):
        return {
            "swe_agent_model_name": args.swe_agent_model_name,
            "swe_agent_per_instance_cost_limit": args.swe_agent_per_instance_cost_limit
        }


    def translate(self):
        """
        1. Generate Translation Task
        """
        self.generate_translation_task()

        """
        2. Copy the original repository to a temporary directory
        3. Initialize the temporary repository and commit the initial state
        """
        self.initialize_temp_repo()

        """
          4. Run the SWE-agent command
          5. Find, rename, and apply the SWE-agent patch file
          6. Save translated output
          7. Remove unnecessary files
          8. Write experiment metadata
          9. Cleanup temporary repository
        """
        try:
            if self.run_swe_agent():
                print("Saving translated output...")
            else:
                print("Translation failed.")
            self.save_output(self._output_path)
            self.remove_unnecessary_output_files()
            self.write_experiment_metadata()
        finally:
            self.cleanup_temp_repo()


    def generate_translation_task(self):
        print("Generating translation task...")

        data = self._dst_config

        translation_task = f"""
You are a helpful coding assistant. You are helping a software developer translate a codebase from the {self._src_model} execution model to the {self._dst_model} execution model.

The codebase is called {data["app"]}. Its path is {data["path"]}. Given this code repository, translate the {data["app"]} codebase's {self._src_model}-specific files to the {self._dst_model} execution model.

The new files should be in {data["filename_desc"]} and all old {self._src_model} files must be deleted. A new {data["build_filename"]} should be made to compile accordingly with the new files.

Ensure that the user can compile this code using, for example, `{data["ex_build_cmd"]}` to build the code for {data["ex_build_desc"]}. Ensure also that the command line interface after translation still works as expected, so that, for example, `{data["ex_run_cmd"]}` still works to run the code with {data["ex_run_desc"]}.
""".strip()

        with open(self._translation_task_path, "w") as f:
            f.write(translation_task)

        print(f"Translation task generated: {self._translation_task_path}")


    def initialize_temp_repo(self):
        """ Initialize the temporary repository
        """
        print("Initializing temporary Git repository...")
        if os.path.exists(self._temp_repo_path):
            print("The temporary repository exists. Removing the repository...")
            shutil.rmtree(self._temp_repo_path)
        # Copies original repo to temp repo
        shutil.copytree(self._input_repo.path, self._temp_repo_path, dirs_exist_ok=True)

        # Initial commits to the temp repo
        subprocess.run(["git", "init"], cwd=self._temp_repo_path, check=True)
        subprocess.run(["git", "add", "."], cwd=self._temp_repo_path, check=True)
        subprocess.run(["git", "commit", "-m", "Initial commit"],
                       cwd=self._temp_repo_path, check=True)


    def run_swe_agent(self):
        """ Run the SWE-agent command
        """
        command = [
            "sweagent", "run",
            f"--agent.model.name={self._swe_agent_model_name}",
            f"--agent.model.per_instance_cost_limit={self._swe_agent_per_instance_cost_limit}",
            f"--env.repo.path={self._temp_repo_path}",
            "--env.deployment.image=python",
            f"--problem_statement.path={self._translation_task_path}",
        ]

        print(f"Running SWE-agent command: {' '.join(command)}")

        try:
            # Runs the SWE-agent command
            process = subprocess.run(command, text=True, cwd=self._temp_repo_path, check=True)
            # If SWE-agent does not encounter any issues, try to apply the patch
            # file via git apply
            if process.returncode == 0:
                print("SWE-agent command executed successfully.")
                print("Applying patch...")

                # The trajectories folder which is created at runtime by
                # SWE-agent. It contains the patch file.
                trajectories_dir = os.path.join(self._temp_repo_path, "trajectories")
                new_patch_path = None

                # Finds the path of the patch file
                def find_patch_file(trajectories_dir):
                    for root, _, files in os.walk(trajectories_dir):
                        for file in files:
                            if file.endswith(".patch"):
                                # Return the full path of the .patch file
                                return os.path.join(root, file)

                    print("Error: No patch file found in trajectories directory.")
                    return None

                old_patch_path = find_patch_file(trajectories_dir)

                # If the old patch path is found, rename the old patch path to
                # the new patch path
                if old_patch_path is None:
                    print("Error: No patch file found in trajectories directory.")
                    return False

                new_patch_path = os.path.join(self._temp_repo_path, "temp.patch")
                os.rename(old_patch_path, new_patch_path)

                # Apply the patch to the temp repo
                try:
                    subprocess.run(["patch", "-p1", "-i", new_patch_path],
                                   cwd=self._temp_repo_path, check=True)
                    print("Patch applied successfully.")
                except subprocess.CalledProcessError as e:
                    print(f"Error applying patch: {e}")
                    return False

                return True

            print(f"Command failed with return code {process.returncode}.")
            # print the subprocess return code
            return False

        except Exception as e:
            print(f"An error occurred: {e}")
            return False


    def save_output(self, output_dir):
        """ Copy the contents of the temporary repository to the final output
            directory. Remove the .git in order to add the output to the results
            repository.
        """
        if os.path.exists(output_dir):
            shutil.rmtree(output_dir)
        shutil.copytree(self._temp_repo_path, output_dir, dirs_exist_ok=True)
        subprocess.run(f"rm -rf {output_dir}/.git", shell=True, check=True)


    def remove_unnecessary_output_files(self):
        """ Remove unnecessary files (any .cu or .cuh files)
        """
        print(f"Cleaning the output repository: {self._output_path}")

        for root, _, files in os.walk(self._output_path):
            for file in files:
                if file.endswith(".cu") or file.endswith(".cuh"):
                    os.remove(os.path.join(root, file))

        print(f"Finished cleaning the output repository: {self._output_path}")


    def write_experiment_metadata(self):
        """ Write an experiment_metadata.json in the output directory with
            details about the translation
        """
        exp_meta_fpath = os.path.join(self._output_path, "..", "experiment_metadata.json")
        os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)

        app_name = os.path.basename(self._input_repo.path)

        with open(exp_meta_fpath, 'w', encoding='utf-8') as f:
            exp_meta_dict = {
                "app": app_name,
                "prompt_strategy": "SWE-agent",
                "llm_name": self._swe_agent_model_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": int(self._output_path.split("/")[-2][7:]),
                "path": self._output_path
            }
            json.dump(exp_meta_dict, f, indent=4)

        print(f"Experiment metadata written to {exp_meta_fpath}.")


    def cleanup_temp_repo(self):
        """ Remove the temporary repository
        """
        print("Cleaning up temporary repository...")
        if os.path.exists(self._temp_repo_path):
            shutil.rmtree(self._temp_repo_path)
        print("Temporary repository cleaned up.")
