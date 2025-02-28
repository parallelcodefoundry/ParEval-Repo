import argparse
import os

def clean_output_repo(translated_repo):
    
    print(f"Cleaning the output repository: {translated_repo}")

    # Remove unnecessary files (any .cu or .cuh files)
    for root, dirs, files in os.walk(translated_repo):
        for file in files:
            if file.endswith(".cu") or file.endswith(".cuh"):
                os.remove(os.path.join(root, file))

    print(f"Finished cleaning the output repository: {translated_repo}")

def get_args():
    parser = argparse.ArgumentParser(description="Clean the output repository translated by SWE-agent and remove unnecessary files")
    parser.add_argument("--translated_repo", type=str, required=True, help="Path to the translated repository")
    return parser.parse_args()

if __name__ == "__main__":
    args = get_args()
    clean_output_repo(args.translated_repo)