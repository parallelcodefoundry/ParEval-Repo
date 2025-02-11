import os
import shutil
import subprocess
import time
from argparse import ArgumentParser

"""Collect arguments from the test_swe-agent_translator.sh script"""
def get_args():
    parser = ArgumentParser(description="Run SWE-agent with a temporary repo and output handling")
    parser.add_argument("--agent_model_name", type=str, required=True, help="Name of the agent model to use")
    parser.add_argument("--per_instance_cost_limit", type=float, required=True, help="Per-instance cost limit for the agent model")
    parser.add_argument("--repo_path", type=str, required=True, help="Path to the original repository")
    parser.add_argument("--deployment_image", type=str, required=True, help="Deployment image name")
    parser.add_argument("--problem_statement_path", type=str, required=True, help="Path to the problem statement file")
    parser.add_argument("--output_dir", type=str, required=True, help="Path to save the translated output")
    return parser.parse_args()

"""Copy the contents of the original repository to a new temporary repository and initialize Git"""
def initialize_temp_repo(original_repo_path, temp_repo_path):
    # If the temp repo exists, clean the repo
    if os.path.exists(temp_repo_path):
        print("The temporary repository exists. Removing the repository...")
        shutil.rmtree(temp_repo_path)
    time.sleep(10)
    # Copies original repo to temp repo
    shutil.copytree(original_repo_path, temp_repo_path, dirs_exist_ok=True)

    # Initial commits to the temp repo
    subprocess.run(["git", "init"], cwd=temp_repo_path, check=True)
    subprocess.run(["git", "add", "."], cwd=temp_repo_path, check=True)
    subprocess.run(["git", "commit", "-m", "Initial commit"], cwd=temp_repo_path, check=True)

"""Run the SWE-agent command on the temporary repository"""
def run_sweagent(args, temp_repo_path):
    # SWE-agent command which will use test_swe-agent_translator.sh script's arguments in terminal
    command = [
        "sweagent", "run",
        f"--agent.model.name={args.agent_model_name}",
        f"--agent.model.per_instance_cost_limit={args.per_instance_cost_limit}",
        f"--env.repo.path={temp_repo_path}",
        f"--env.deployment.image={args.deployment_image}",
        f"--problem_statement.path={args.problem_statement_path}"
    ]

    print(f"Running command: {' '.join(command)}")

    try:
        # Runs the SWE-agent command
        process = subprocess.run(command, text=True, cwd=temp_repo_path)
        # If SWE-agent does not encounter any issues, try to apply the patch file via git apply
        if process.returncode == 0:
            print("SWE-agent command executed successfully.")
            time.sleep(20)
            print("Applying patch...")

            # The trajectories folder which is created at runtime by SWE-agent. It contains the patch file.
            trajectories_dir = "/Users/ishan/pssg/tmp/temp_sweagent_repo/trajectories/ishan"
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
                new_patch_path = "/Users/ishan/pssg/tmp/temp_sweagent_repo/temp.patch"
                os.rename(old_patch_path, new_patch_path)

                # # Add the patch file to Git in the temp repo
                # subprocess.run(["git", "add", "-A"], cwd=temp_repo_path, check=True)
                # subprocess.run(["git", "commit", "-m", "Added patch file"], cwd=temp_repo_path, check=True)

            time.sleep(10)
            # Clears the unnecessary whitespace of the patch file
            subprocess.run(["sed", "-i", "", "s/[ \t]*$//", new_patch_path], check=True, cwd=temp_repo_path) # update regex if needed
            print('Cleared up the whitespace of the patch file')

            time.sleep(10)
            
            # Apply the patch to the temp repo
            try:
                subprocess.run(["git", "apply", "--directory=.", "--reject", "--whitespace=fix", "--unsafe-paths", new_patch_path], cwd=temp_repo_path, check=True)
                print("Patch applied successfully.")
                
            except subprocess.CalledProcessError as e:
                time.sleep(20)
                return False

            return True
        
        else:
            print(f"Command failed with return code {process.returncode}.")
            # print the subprocess return code
            return False

    except Exception as e:
        print(f"An error occurred: {e}")
        return False

"""Copy the contents of the temporary repository to the output directory"""
def save_output(temp_repo_path, output_dir):
    # If the output repo exists, clean the repo
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    shutil.copytree(temp_repo_path, output_dir, dirs_exist_ok=True)
    subprocess.run(f"rm -rf {args.output_dir}/.git", shell=True, check=True)

if __name__ == "__main__":
    args = get_args()

    temp_repo_path = "/Users/ishan/pssg/tmp/temp_sweagent_repo"
    
    print("Initializing temporary Git repository...")
    initialize_temp_repo(args.repo_path, temp_repo_path)

    try:
        if run_sweagent(args, temp_repo_path):
            print("Saving translated output...")
        else:
            print("Translation failed. No output saved.")
        save_output(temp_repo_path, args.output_dir)

    finally:
        print("Cleaning up temporary repository...")
        if os.path.exists(temp_repo_path):
            shutil.rmtree(temp_repo_path)
        print("Temporary repository cleaned up.")

        # subprocess.run(["git", "add", "-A"], check=True)
        # subprocess.run(["git", "commit", "-m", "Added " + args.output_dir], check=True)