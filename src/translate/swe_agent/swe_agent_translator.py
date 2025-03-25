# std imports
import os
import shutil
import subprocess
import json

# local imports
from translator import Translator
from repo import Repo

class SWEAgentTranslator(Translator):
    """
    Class that invokes SWE-agent to perform code translation
    """

    def __init__(
        self,
        input_repo: Repo,
        output_repo: os.PathLike,
        src_model: str,
        dst_model: str,
        dst_config: dict,
        log_interactions=False,
        dry=False,
        hide_progress=False,
        swe_agent_model_name=None,
        swe_agent_per_instance_cost_limit=0.10,
        swe_agent_deployment_image=None,
        swe_agent_problem_statement_path=None,
        swe_agent_output_id=None,
    ):
        super().__init__(
            input_repo,
            output_repo,
            src_model,
            dst_model,
            dst_config,
            log_interactions=log_interactions,
            dry=dry,
            hide_progress=hide_progress
        )

        self.swe_agent_model_name = swe_agent_model_name
        self.swe_agent_per_instance_cost_limit = swe_agent_per_instance_cost_limit
        self.swe_agent_deployment_image = swe_agent_deployment_image
        self.swe_agent_problem_statement_path = swe_agent_problem_statement_path
        self.swe_agent_output_id = swe_agent_output_id
        self.temp_repo_path = "/tmp/temp_sweagent_repo"

    @staticmethod
    def add_args(parser):
        parser.add_argument("--swe-agent-model-name", type=str, required=True,
                            help="Name of the agent model to use (e.g. 'gpt-4o').")
        parser.add_argument("--swe-agent-per-instance-cost-limit", type=float, required=True,
                            help="Per-instance cost limit for the agent model.")
        parser.add_argument("--swe-agent-deployment-image", type=str, required=True,
                            help="Deployment image name for the environment.")
        parser.add_argument("--swe-agent-problem-statement-path", type=str, required=True,
                            help="Path to the problem statement file for the agent.")
        parser.add_argument("--swe-agent-output-id", type=int, required=True,
                            help="Integer ID for this run (used in experiment metadata).")
    
    @staticmethod
    def parse_args(args):
        return {
            "swe_agent_model_name": args.swe_agent_model_name,
            "swe_agent_per_instance_cost_limit": args.swe_agent_per_instance_cost_limit,
            "swe_agent_deployment_image": args.swe_agent_deployment_image,
            "swe_agent_problem_statement_path": args.swe_agent_problem_statement_path,
            "swe_agent_output_id": args.swe_agent_output_id,
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
            self.save_output(self.output_repo)
            self.remove_unnecessary_output_files()
            self.write_experiment_metadata()
        finally:
            self.cleanup_temp_repo()

    def generate_translation_task(self):
        print("Generating translation task...")

        target_json = os.path.join(self.input_repo.path, "target.json")
        translation_task_path = os.path.join(self.input_repo.path, "translation_task.md")

        with open(target_json, "r") as f:
            data = json.load(f)

        translation_task = f"""
        You are a helpful coding assistant.
        You are helping a software developer translate a codebase from the {data["model"]} execution model to the {self._dst_model} execution model.
        The codebase is called {data["app"]}. Its path is {data["path"]}.
        Given this code repository, translate the {data["app"]} codebase's {data["model"]} files to the {self._dst_model} execution model.
        The new files should be in {data["filename_desc"]} and all old {data["model"]} files must be deleted. A new Makefile should be made to compile accordingly with the new files.
        """.strip()
        
        with open(translation_task_path, "w") as f:
            f.write(translation_task)

        print(f"Translation task generated: {translation_task_path}")

    def initialize_temp_repo(self):
        """
        Initialize the temporary repository
        """
        print("Initializing temporary Git repository...")
        if os.path.exists(self.temp_repo_path):
            print("The temporary repository exists. Removing the repository...")
            shutil.rmtree(self.temp_repo_path)
        # Copies original repo to temp repo
        shutil.copytree(self.input_repo, self.temp_repo_path, dirs_exist_ok=True)

        # Initial commits to the temp repo
        subprocess.run(["git", "init"], cwd=self.temp_repo_path, check=True)
        subprocess.run(["git", "add", "."], cwd=self.temp_repo_path, check=True)
        subprocess.run(["git", "commit", "-m", "Initial commit"], cwd=self.temp_repo_path, check=True)
    
    def run_swe_agent(self):
        """
        Run the SWE-agent command
        """
        command = [
            "sweagent", "run",
            f"--agent.model.name={self.swe_agent_model_name}",
            f"--agent.model.per_instance_cost_limit={self.swe_agent_per_instance_cost_limit}",
            f"--env.repo.path={self.temp_repo_path}",
            f"--env.deployment.image={self.swe_agent_deployment_image}",
            f"--problem_statement.path={self.swe_agent_problem_statement_path}",
        ]

        print(f"Running SWE-agent command: {' '.join(command)}")

        try:
            # Runs the SWE-agent command
            process = subprocess.run(command, text=True, cwd=self.temp_repo_path)
            # If SWE-agent does not encounter any issues, try to apply the patch file via git apply
            if process.returncode == 0:
                print("SWE-agent command executed successfully.")
                print("Applying patch...")

                # The trajectories folder which is created at runtime by SWE-agent. It contains the patch file.
                trajectories_dir = "/Users/ishan/pssg/tmp/temp_sweagent_repo/trajectories/ishan" # @TODO: replace with actual path (os.join(self.temp_repo_path, "trajectories"))
                # The new patch which the patch file will be renamed to
                new_patch_path = None

                # Finds the path of the patch file
                def find_patch_file(trajectories_dir):
                    for root, dirs, files in os.walk(trajectories_dir):
                        for file in files:
                            if file.endswith(".patch"):
                                return os.path.join(root, file)  # Return the full path of the .patch file
                            
                    print("Error: No patch file found in trajectories directory.")
                    return None
                
                old_patch_path = find_patch_file(trajectories_dir)

                # If the old patch path is found, rename the old patch path to the new patch path
                if old_patch_path is None:
                    print("Error: No patch file found in trajectories directory.")
                    return False
                else:
                    new_patch_path = "/Users/ishan/pssg/tmp/temp_sweagent_repo/temp.patch" # @TODO: replace with actual path (os.join(self.temp_repo_path, "temp.patch"))
                    os.rename(old_patch_path, new_patch_path)

                # Clears the unnecessary whitespace of the patch file
                subprocess.run(["sed", "-i", "", "s/[ \t]*$//", new_patch_path], check=True, cwd=self.temp_repo_path)
                print('Cleared up the whitespace of the patch file')
                
                # Apply the patch to the temp repo
                try:
                    subprocess.run(["git", "apply", "--directory=.", "--reject", "--whitespace=fix", "--unsafe-paths", new_patch_path], cwd=self.temp_repo_path, check=True)
                    print("Patch applied successfully.")
                    
                except subprocess.CalledProcessError as e:
                    return False

                return True
            
            else:
                print(f"Command failed with return code {process.returncode}.")
                # print the subprocess return code
                return False

        except Exception as e:
            print(f"An error occurred: {e}")
            return False

    def save_output(self, output_dir):
        """
        Copy the contents of the temporary repository to the final output directory.
        Remove the .git in order to add the output to the results repository.
        """
        if os.path.exists(output_dir):
            shutil.rmtree(output_dir)
        shutil.copytree(self.temp_repo_path, output_dir, dirs_exist_ok=True)
        subprocess.run(f"rm -rf {output_dir}/.git", shell=True, check=True)

    def remove_unnecessary_output_files(self):
        """
        Remove unnecessary files (any .cu or .cuh files)
        """
        print(f"Cleaning the output repository: {self.output_repo}")

        for root, _, files in os.walk(self.output_repo):
            for file in files:
                if file.endswith(".cu") or file.endswith(".cuh"):
                    os.remove(os.path.join(root, file))

        print(f"Finished cleaning the output repository: {self.output_repo}")
    
    def write_experiment_metadata(self):
        """
        Write an experiment_metadata.json in the output directory with
        details about the translation
        """
        exp_meta_fpath = os.path.join(self.output_repo, "experiment_metadata.json")
        os.makedirs(os.path.dirname(exp_meta_fpath), exist_ok=True)

        app_name = os.path.basename(self._input_repo.path)

        with open(exp_meta_fpath, 'w') as f:
            exp_meta_dict = {
                "app": app_name,
                "prompt_strategy": ["SWE", "agent"],
                "llm_name": self.swe_agent_model_name,
                "source_model": self._src_model,
                "dest_model": self._dst_model,
                "output_number": self.swe_agent_output_id,
                "path": self._output_repo
            }
            json.dump(exp_meta_dict, f, indent=4) 

        print(f"Experiment metadata written to {exp_meta_fpath}.")    

    def cleanup_temp_repo(self):
        """
        Remove the temporary repository
        """
        print("Cleaning up temporary repository...")
        if os.path.exists(self.temp_repo_path):
            shutil.rmtree(self.temp_repo_path)
        print("Temporary repository cleaned up.")