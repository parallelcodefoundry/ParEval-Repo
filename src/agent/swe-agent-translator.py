import os
import shutil
import subprocess
from argparse import ArgumentParser

def get_args():
    parser = ArgumentParser(description="Run SWE-agent for CUDA to OpenMP translation using Ollama")
    parser.add_argument("--repo_path", type=str, required=True, help="Path to the temporary repository")
    parser.add_argument("--output_base_dir", type=str, required=True, help="Base directory for the output")
    parser.add_argument("--keys_cfg", type=str, required=True, help="Path to the keys.cfg file")
    parser.add_argument("--config_file", type=str, required=True, help="Path to the config.yaml file")
    parser.add_argument("--data_path", type=str, required=True, help="Path to the data file")
    parser.add_argument("--model_name", type=str, help="Name of the Ollama model to use")
    parser.add_argument("--host_url", type=str, help="URL of the Ollama server")
    parser.add_argument("--per_instance_cost_limit", type=float, required=True, help="Cost limit per instance")
    parser.add_argument("--input_directory", type=str, required=True, help="Path to the input source directory")
    return parser.parse_args()

"""Sets up a temporary Git repository with the contents of the CUDA directory"""
def initialize_temp_repo(repo_path, input_directory):
    if os.path.exists(repo_path):
        shutil.rmtree(repo_path)
    os.makedirs(repo_path)

    # Copy contents of the input directory to the temporary repo
    shutil.copytree(input_directory, repo_path, dirs_exist_ok=True)

    # Initialize Git repository
    subprocess.run(["git", "init"], cwd=repo_path, check=True)
    subprocess.run(["git", "add", "."], cwd=repo_path, check=True)
    subprocess.run(["git", "commit", "-m", "Initial commit"], cwd=repo_path, check=True)

"""Runs SWE-agent for a single iteration, and stores log outputs and file modifications"""
def run_swe_agent(iteration, args):
    output_dir = os.path.join(args.output_base_dir, f"output-{iteration}")
    
    # Deletes the output directory if it exists to avoid overwriting issues
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir, exist_ok=True)

    log_file_path = os.path.join(output_dir, f"output-{iteration}-swe-agent-comments.txt")

    command = [
        "docker", "run", "--rm", "-it", "-v", "/var/run/docker.sock:/var/run/docker.sock",
        "-v", f"{args.keys_cfg}:/app/keys.cfg",
        "-v", f"{args.repo_path}:{args.repo_path}",
        "-v", "/Users/ishan/pssg/SWE-agent:/Users/ishan/pssg/SWE-agent",
        "-v", "/Users/ishan/pssg/code-translation:/Users/ishan/pssg/code-translation",
        "sweagent/swe-agent-run:latest",
        "python", "/Users/ishan/pssg/SWE-agent/run.py",
        "--image_name=sweagent/swe-agent:latest",
        f"--model_name={args.model_name}",
        f"--host_url={args.host_url}",
        f"--data_path={args.data_path}",
        f"--repo_path={args.repo_path}",
        f"--config_file={args.config_file}",
        "--apply_patch_locally",
        f"--per_instance_cost_limit={args.per_instance_cost_limit}"
    ]

    print(f"Running iteration {iteration}...")

    # Run the command and capture output
    try:
        process = subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # Log output after the command completes
        with open(log_file_path, "w") as log_file:
            log_file.write(f"--- Iteration {iteration} ---\n")
            log_file.write("--- STDOUT ---\n")
            log_file.write(process.stdout + "\n")
            log_file.write("--- STDERR ---\n")
            log_file.write(process.stderr + "\n")

        if process.returncode != 0:
            print(f"Error in iteration {iteration}. Check {log_file_path} for details.")
        else:
            print(f"Iteration {iteration} completed successfully. Output saved to {log_file_path}.")

    except Exception as e:
        with open(log_file_path, "a") as log_file:
            log_file.write(f"EXCEPTION: {str(e)}\n")
        print(f"Exception occurred during iteration {iteration}. Check {log_file_path} for details.")

    # Copy the modified repo to the output directory
    shutil.copytree(args.repo_path, output_dir, dirs_exist_ok=True)

"""Resets the Git repository to its original state"""
def reset_repo(repo_path):
    subprocess.run(["git", "reset", "--hard", "HEAD"], cwd=repo_path, check=True)

if __name__ == "__main__":
    args = get_args()

    print("Initializing temporary Git repository...")
    initialize_temp_repo(args.repo_path, args.input_directory)

    try:
        run_swe_agent(0, args)
    except subprocess.CalledProcessError as e:
        print(f"Error during iteration 0: {e}")
    finally:
        print("Resetting repository...")
        reset_repo(args.repo_path)