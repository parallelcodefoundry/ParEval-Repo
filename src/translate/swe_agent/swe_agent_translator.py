import os
import shutil
import subprocess
import time
import json
from ..translator import Translator

class SWEAgentTranslator(Translator):
    """
    Class that invokes SWE-agent to perform code translation
    """

    def __init__(
        self,
        input_repo,
        output_repo,
        src_model,
        dst_model,
        dst_config,
        log_interactions=False,
        dry=False,
        hide_progress=False,
        swe_agent_model_name=None,
        swe_agent_per_instance_cost_limit=2.0,
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
        self._temp_repo_path = "/tmp/temp_sweagent_repo"

    @staticmethod
    def add_args(parser):
        parser.add_argument("--swe-agent-model-name", type=str, required=True,
                            help="Name of the agent model to use (e.g. 'gpt-4').")
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
        To Do's
          1. Copy the original repo to a temp directory
          2. Initialize git and commit
          3. Run the SWE-agent command
          4. Find/rename/cleanup/apply patch
          5. Copy the result to the final output directory (Partially done)
          6. Write experiment metadata (Done, check the repo path part since it's not a string)
          7. Cleanup (Done)
        """
        

    def _save_output(self, temp_repo_path, output_dir):
        """
        Copy the contents of the temporary repository to the final output directory.
        Remove the .git in order to add the output to the results repository.
        """
        if os.path.exists(output_dir):
            shutil.rmtree(output_dir)
        shutil.copytree(temp_repo_path, output_dir, dirs_exist_ok=True)
        # Remove the .git directory
        # Change later: subprocess.run(f"rm -rf {args.output_dir}/.git", shell=True, check=True)
    
    def _write_experiment_metadata(self):
        """
        Write an experiment_metadata.json in the output directory with
        details about the translation
        """
        exp_meta_fpath = os.path.join(self._output_repo, "experiment_metadata.json")
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

    def _cleanup_temp_repo(self):
        """
        Remove the temporary repository
        """
        if os.path.exists(self._temp_repo_path):
            shutil.rmtree(self._temp_repo_path)
        print("Temporary repository cleaned up.")