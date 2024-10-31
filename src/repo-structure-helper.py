#!/usr/bin/env python3
'''
This script is used to change the format of each output repository in the provided directory
to include a meta.json file that contains the metadata of the repository and a repo folder
that contains all the repo files. The metadata is extracted from the provided existing metadata
file from the target directory as well as the directory names in the provided directory.
'''

import os
import sys
import json

# Function to rename files using git mv
def git_rename(src, dst):
    os.system(f'git mv {src} {dst}')
    print(f'git mv {src} {dst}')

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 data-adjust-helper.py <input-dir> <metadata-file>")
        sys.exit(1)

    input_dir = sys.argv[1]
    metadata_file = sys.argv[2]

    with open(metadata_file, 'r') as f:
        input_metadata = json.load(f)

    # get outer metadata based on root directory name
    full_path = os.path.abspath(input_dir)
    prompt_strategy, llm_name, source_model, _, target_model = full_path.split('/')[-1].split('-')
    print(f"Prompt Strategy: {prompt_strategy}")
    print(f"LLM Name: {llm_name}")
    print(f"Source Model: {source_model}")
    print(f"Target Model: {target_model}")

    # validate that target_model in metadata matches target_model in directory name
    if input_metadata["model"] != target_model:
        raise ValueError(f"Target model in metadata file ({input_metadata['target_model']}) does not match target model in directory name ({target_model})")

    for repo in os.listdir(input_dir):
        repo_path = os.path.join(input_dir, repo)

        # get output number based on directory name
        output_number = repo.split('-')[-1]

        metadata = {
            "app": input_metadata["app"],
            "prompt_strategy": prompt_strategy,
            "llm_name": llm_name,
            "source_model": source_model,
            "target_model": target_model,
            "output_number": output_number,
            "path": repo_path
        }

        if os.path.isdir(repo_path):
            repo_files = os.listdir(repo_path)
            if 'meta.json' in repo_files:
                repo_files.remove('meta.json')
            if 'repo' in repo_files:
                repo_files.remove('repo')
            os.makedirs(os.path.join(repo_path, 'repo'), exist_ok=True)
            for repo_file in repo_files:
                git_rename(os.path.join(repo_path, repo_file), os.path.join(repo_path, 'repo', repo_file))
            with open(os.path.join(repo_path, 'meta.json'), 'w') as f:
                json.dump(metadata, f, indent=4)
                print(json.dumps(metadata, indent=4))
                print(f"Writen to {os.path.join(repo_path, 'meta.json')}")

        print(f"Processed {repo}\n------------------------")

if __name__ == "__main__":
    main()
