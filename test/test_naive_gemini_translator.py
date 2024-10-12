import os
import sys
import time
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src'))
from repo import Repo
from naive.naive_gemini_translator import NaiveGeminiTranslator

def main(output_path):
    input_path = "/Users/ishan/pssg/code-translation/targets/microXOR/cuda/"
    src_model = "cuda"
    dst_model = "openmp-offload"

    input_repo = Repo(input_path)

    translator = NaiveGeminiTranslator(input_repo, output_path, src_model, dst_model)

    translator.translate(dry=False)

if __name__ == "__main__":
    for i in range(5):
        time.sleep(15)
        print('Making Output #' + str(i))
        output_path = "/Users/ishan/pssg/code-translation/data/microXOR/naive-gemini-cuda-to-omp/output-" + str(i)
        main(output_path)