""" Naively translate microXOR using Gemini from CUDA to OpenMP Offload
"""

import os
import re
import requests
import google.generativeai as genai

# these are the CUDA files which will be transformed into OpenMP Offload
cuda_files = ['/Users/ishan/pssg/code-translation/targets/microXOR/cuda/Makefile',
              '/Users/ishan/pssg/code-translation/targets/microXOR/cuda/include/microXOR.cuh',
              '/Users/ishan/pssg/code-translation/targets/microXOR/cuda/src/main.cu',
              '/Users/ishan/pssg/code-translation/targets/microXOR/cuda/src/microXOR.cu']

# the output folder collection in data/microXOR
output_folder = '/Users/ishan/pssg/code-translation/data/microXOR/naive-gemini-cuda-to-omp'

# reads the cuda files
def read_cuda_file(file_path):
    with open(file_path, 'r') as file:
        return file.read()

def translate_cuda_to_openmp():

    naive_prompt = """
    Below is a stencil computation benchmark computing an XOR operation over a 2D grid of cells, called microXOR. 
    This version of microXOR is written in CUDA for GPU execution.
    We are translating microXOR to OpenMP Offload.
    Here is the code for microXOR:

    {microXOR_code}

    Translate all three CUDA files to OpenMP Offload and update the Makefile if necessary. 
    The new files should be in C++ (.cpp or .hpp files), and any CUDA content should be substituted with OpenMP Offload. 
    Lastly, output all four newly-translated files as follows for each file:

    File: file_name

    # File's Code

    """

    # Step 1: Read the CUDA code
    cummulative_microXOR_code = ""
    for cuda_file in cuda_files:
        if os.path.exists(cuda_file):
            microXOR_code = read_cuda_file(cuda_file)
            cummulative_microXOR_code += microXOR_code + "\n"
        else:
            print(f"File {cuda_file} does not exist.")

    updated_prompt = naive_prompt.format(microXOR_code=cummulative_microXOR_code)

    # Step 2: Make a request to Gemini's API to translate the code
    genai.configure(api_key=os.environ["API_KEY"])
    model = genai.GenerativeModel("gemini-1.5-flash")

    response = model.generate_content(updated_prompt)
    comments_text = (response.text)[response.text.rfind("```") + 4:]
    response_text = (response.text)[:response.text.rfind("```") + 3]
    # print(response_text)

    # Step 3: Write the translated code to a new output file
    os.makedirs(output_folder, exist_ok=True)

    pattern = r'## File: (.+?)\n(.*?)(?=\n## File:|$)'

    matches = re.findall(pattern, response_text, re.DOTALL)

    for file_name, code in matches:
        
        # Update file_name if necessary
        if file_name.strip() == "microXOR.hpp":
            file_name = "include/microXOR.hpp"
        elif file_name.strip() == "microXOR.cpp" or file_name.strip() == "main.cpp":
            file_name = "src/" + file_name.strip()

        print(file_name.strip())

        output_file_path = os.path.join(output_folder, file_name.strip())
        os.makedirs(os.path.dirname(output_file_path), exist_ok=True)
        if file_name != "Makefile":
            # removes starting ```
            code = code[8:]
            # removes ending ```
            code = code[:len(code) - 4]
        else:
            # removes starting ```
            code = code[13:]
            # removes ending ```
            code = code[:len(code) - 3]

        with open(output_file_path, 'w') as output_file:
            output_file.write(code.strip())
        
        print(f"Wrote {file_name.strip()} to {output_file_path}")

    # Store Gemini's comments in a textfile
    comments_file_path = os.path.join(output_folder, "comments.txt")

    # Write the comments to the text file
    with open(comments_file_path, 'w') as comments_file:
        comments_file.write(comments_text.strip())

    print(f"Wrote comments to {comments_file_path}")

for i in range(0, 5):
    new_folder = "/output-" + str(i)
    output_folder += new_folder
    translate_cuda_to_openmp()
    output_folder = output_folder[:len(output_folder) - 9]