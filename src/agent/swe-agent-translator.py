import os
import shutil
import subprocess
import time

REPO_PATH = "/Users/ishan/tmp/microXOR_cuda_repo"
OUTPUT_BASE_DIR = "/Users/ishan/pssg/code-translation/data/microXOR/SWE-agent-cuda-to-omp"
SWE_AGENT_IMAGE = "sweagent/swe-agent-run:latest"
KEYS_CFG = "/Users/ishan/pssg/SWE-agent/keys.cfg"
CONFIG_FILE = "/Users/ishan/pssg/SWE-agent/config/default.yaml"
DATA_PATH = "/Users/ishan/pssg/code-translation/targets/microXOR/translation_task.md"
MODEL_NAME = "gpt4omini"
PER_INSTANCE_COST_LIMIT = 0.50

"""Sets up a temporary Git repository with the contents of the CUDA directory"""
def initialize_temp_repo():
    if os.path.exists(REPO_PATH):
        shutil.rmtree(REPO_PATH)
    os.makedirs(REPO_PATH)

    # Copy contents of microXOR/cuda directory to the temporary repo
    shutil.copytree("/Users/ishan/pssg/code-translation/targets/microXOR/cuda", REPO_PATH, dirs_exist_ok=True)

    # Initialize Git repository
    subprocess.run(["git", "init"], cwd=REPO_PATH, check=True)
    subprocess.run(["git", "add", "."], cwd=REPO_PATH, check=True)
    subprocess.run(["git", "commit", "-m", "Initial commit"], cwd=REPO_PATH, check=True)

"""Runs SWE-agent for a single iteration, and stores log outputs and file modifications"""
def run_swe_agent(iteration):
    output_dir = os.path.join(OUTPUT_BASE_DIR, f"output-{iteration}")
    
    # Deletes the output directory if it exists to avoid overwriting issues
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir, exist_ok=True)

    log_file_path = os.path.join(OUTPUT_BASE_DIR, f"output-{iteration}/output-{iteration}-swe-agent-comments.txt")

    """
    Sample SWE-agent command that will execute:

    docker run --rm -it -v /var/run/docker.sock:/var/run/docker.sock \
    -v /Users/ishan/pssg/SWE-agent/keys.cfg:/app/keys.cfg \
    -v /tmp/microXOR_cuda_repo:/tmp/microXOR_cuda_repo \
    sweagent/swe-agent-run:latest \
    python /Users/ishan/pssg/SWE-agent/run.py --image_name=sweagent/swe-agent:latest \
    --model_name gpt4omini \ 
    --data_path /Users/ishan/pssg/code-translation/targets/microXOR/translation_task.md \
    --repo_path /tmp/microXOR_cuda_repo \
    --config_file /Users/ishan/pssg/SWE-agent/config/default.yaml \
    --apply_patch_locally \
    --per_instance_cost_limit 0.50

    """

    command = [
        "docker", "run", "--rm", "-it", "-v", "/var/run/docker.sock:/var/run/docker.sock",
        "-v", f"{KEYS_CFG}:/app/keys.cfg",
        "-v", f"{REPO_PATH}:{REPO_PATH}",
        "-v", "/Users/ishan/pssg/SWE-agent:/Users/ishan/pssg/SWE-agent",
        "-v", "/Users/ishan/pssg/code-translation:/Users/ishan/pssg/code-translation",
        SWE_AGENT_IMAGE,
        "python", "/Users/ishan/pssg/SWE-agent/run.py", "--image_name=sweagent/swe-agent:latest",
        f"--model_name={MODEL_NAME}",
        f"--data_path={DATA_PATH}",
        f"--repo_path={REPO_PATH}",
        f"--config_file={CONFIG_FILE}",
        "--apply_patch_locally",
        f"--per_instance_cost_limit={PER_INSTANCE_COST_LIMIT}"
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

        print('done with command')

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
    shutil.copytree(REPO_PATH, output_dir, dirs_exist_ok=True)

"""Resets the Git repository to its original state"""
def reset_repo():
    subprocess.run(["git", "reset", "--hard", "HEAD"], cwd=REPO_PATH, check=True)

# Initialize the temporary Git repository
print("Initializing temporary Git repository...")
initialize_temp_repo()

try:
    run_swe_agent(0)
except subprocess.CalledProcessError as e:
    print(f"Error during iteration {0}: {e}")
finally: # Reset the repository after each iteration
    print(f"Resetting repository for iteration {0 + 1}...")
    reset_repo()
    time.sleep(30)

print("All iterations complete.")