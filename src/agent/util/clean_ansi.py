import re
import argparse

def clean_ansi_escape_sequences(input_file, output_file):
    """
    Remove ANSI escape sequences from the input file and save the cleaned content to the output file.
    """
    ansi_escape = re.compile(r'\x1B\[[0-9;]*[a-zA-Z]')
    
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        for line in infile:
            cleaned_line = ansi_escape.sub('', line)  # Remove escape sequences
            outfile.write(cleaned_line)

def get_args():
    """
    Parse command-line arguments for the input and output file paths.
    """
    parser = argparse.ArgumentParser(description="Clean the ANSI escape sequences from an input file")
    parser.add_argument("--input_file", type=str, required=True, help="Path to the input file")
    parser.add_argument("--output_file", type=str, required=True, help="Path to the new output file")
    return parser.parse_args()

if __name__ == "__main__":
    args = get_args()
    clean_ansi_escape_sequences(args.input_file, args.output_file)
    print(f"New file: {args.output_file}")