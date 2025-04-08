#!/usr/bin/env python3
"""
json-merger.py
A script to merge multiple JSON files into one.

Usage: python json-merger.py <output_file.json> <input_file1.json> <input_file2.json> ...
"""

import json
import sys
import os

def merge_dicts(dict1, dict2):
    """ Merge two dictionaries. Expects the keys in both dicts to be integers
        (stored as strings) and if a key exists in both, the value under that
        key dict2 will be added to the resulting dictionary with a unique value.
        Returns the merged dictionary.
    """

    # If either dictionary is empty, return the other
    if len(dict1) == 0:
        return dict2
    if len(dict2) == 0:
        return dict1

    # Find the maximum key in dict1 to generate new keys for dict2
    max_key = max((int(k) for k in dict1.keys()))

    # Create a new dictionary to hold the merged result
    result = dict1.copy()

    # Iterate through dict2 and add its items to the result
    for key, value in dict2.items():
        if key in result.keys():
            # If the key exists in both, choose a new unique key for dict2 data
            new_key = str(int(key) + max_key + 1)
            result[new_key] = value
        else:
            # If the key doesn't exist in dict1, just add it
            result[key] = value

    return result

def merge_json_files(output_file, input_files):

    """ Merge multiple JSON files into a single JSON file.
    """
    merged_data = {}

    for input_file in input_files:
        if not os.path.isfile(input_file):
            print(f"Error: {input_file} does not exist.")
            continue

        with open(input_file, 'r', encoding='utf-8') as f:
            try:
                data = json.load(f)
                merged_data = merge_dicts(merged_data, data)
            except json.JSONDecodeError as e:
                print(f"Error decoding JSON from {input_file}: {e}")

    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(merged_data, f, indent=4)

    print(f"Merged JSON files into {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python json-merger.py <output_file.json> "
              + "<input_file1.json> <input_file2.json> ...")
        sys.exit(1)

    output_fpath = sys.argv[1]
    input_fpaths = sys.argv[2:]

    # Check if output file already exists
    if os.path.isfile(output_fpath):
        raise FileExistsError(f"{output_fpath} already exists. Please choose a different name.")

    merge_json_files(output_fpath, input_fpaths)
