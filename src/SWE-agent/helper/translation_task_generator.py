import json
import argparse

def generate_translation_task(target_json, translation_task_file):
    with open(target_json, "r") as f:
        data = json.load(f)

    translation_task = f"""
You are a helpful coding assistant.

You are helping a software developer translate a codebase from the {data["model"]} execution model to the OpenMP Offload execution model.

The codebase is called {data["app"]}. Its path is {data["path"]}.

Given this code repository, translate the {data["app"]} codebase's {data["model"]} files to the OpenMP Offload execution model.

The new files should be in {data["filename_desc"]} and all old {data["model"]} files must be deleted. A new Makefile should be made to compile accordingly with the new files.
""".strip()

    with open(translation_task_file, "w") as f:
        f.write(translation_task)

    print(f"Translation task generated: {translation_task_file}")

def get_args():
    parser = argparse.ArgumentParser(description="Generate a translation task md file based on a given target.json file")
    parser.add_argument("--target_json", type=str, required=True, help="Path to the target.json file")
    parser.add_argument("--output_file", type=str, required=True, help="Path to save the generated translation_task.md file")
    return parser.parse_args()

if __name__ == "__main__":
    args = get_args()
    generate_translation_task(args.target_json, args.output_file)