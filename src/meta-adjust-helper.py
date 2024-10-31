#!/usr/bin/env python3
'''
This script is used to systematically edit the contents of all meta.json files
in the provided directory root.
'''

import os
import sys
import json

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 meta-adjust-helper.py <directory>")
        sys.exit(1)

    root = sys.argv[1]
    for subdir, dirs, files in os.walk(root):
        for file in files:
            if file == "meta.json":
                path = os.path.join(subdir, file)
                with open(path, 'r') as f:
                    data = json.load(f)

                    # Edit the contents of the meta.json file here
                    print("Editing", path)
                    print("Old path:", data["path"])
                    print("New path:", os.path.join(data["path"], "repo"))
                    data["path"] = os.path.join(data["path"], "repo")

                print("Writing to", path)
                with open(path, 'w') as f:
                    json.dump(data, f, indent=4)

if __name__ == "__main__":
    main()
