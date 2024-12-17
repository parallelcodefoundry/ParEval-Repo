#!/usr/bin/env python3
""" Translating repositories from one execution model to another using LLMs.
    For example, this script can take a CUDA application as input and 
    translate it to an OpenMP version of the same application.

    author: Daniel Nichols
    date: April 2024
"""
# std imports
from argparse import ArgumentParser
import os
import json

# local imports
from repo import Repo
from translator import Translator
from naive.naive_openai_translator import NaiveOpenAITranslator
from naive.naive_gemini_translator import NaiveGeminiTranslator
from naive.naive_ollama_translator import NaiveOllamaTranslator
#from naive.naive_vllm_translator import NaiveVLLMTranslator

def get_args():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("-i", "--input", type=str, required=True, help="Path to the input source code repository.")
    parser.add_argument("-o", "--output", type=str, required=True, help="Path to the output source code repository.")
    parser.add_argument("-c", "--config", type=str, required=True, help="Path to translation destination model configuration file containing prompt fill-ins.")
    parser.add_argument("-f", "--force-overwrite", action="store_true", help="Force overwrite of existing output directory.")
    parser.add_argument("--method", choices=["naive"], required=True, help="The translation method to use.")
    parser.add_argument("--src-model", type=str, required=True, help="The source execution model.")
    parser.add_argument("--dst-model", type=str, required=True, help="The destination execution model.")
    parser.add_argument("--output-id", type=int, help="The integer ID of the output, used to count repeat instances of the same translation configuration.")
    parser.add_argument("--app-name", type=str, help="The name of the application being translated.")
    parser.add_argument("--dry", "-d", action="store_true", help="Dry run the translation.")
    parser.add_argument("--log-interactions", action="store_true", help="Log the raw LLM outputs to a text file.")
    parser.add_argument("--hide-progress", action="store_true", help="Hide the progress bar.")

    # subgroup of arguments for the naive translation method
    naive_args = parser.add_argument_group("naive translation method")
    naive_args.add_argument("--naive-llm", choices=["gpt-3.5", "gpt-4o-mini", "gemini", "llama3.3", "llama3.2", "llama3.1", "gpt-4o"], default="gemini", help="The LLM to use for translation.")
    return parser.parse_args()


def get_translator_cls(method: str, naive_llm: str):
    if method == "naive":
        if naive_llm == "gemini":
            return NaiveGeminiTranslator
        if "llama" in naive_llm:
            return NaiveOllamaTranslator
        else:
            return NaiveOpenAITranslator
    else:
        raise ValueError(f"Translation method {method} not recognized.")


def main():
    args = get_args()

    # check if the input directory exists
    if not os.path.exists(args.input):
        raise FileNotFoundError(f"Input directory {args.input} not found.")

    # check if the output directory exists and is empty
    if os.path.exists(args.output):
        output_dir = os.path.join(args.output, "output-" + str(args.output_id))
        if os.path.exists(output_dir) and not args.force_overwrite:
            raise FileExistsError(f"Output directory {output_dir} already exists. Provide --force-overwrite to overwrite.")
    else:
        os.mkdir(args.output)

    # check that the dest model target json for prompt config exists
    if not os.path.exists(args.config):
        raise FileNotFoundError(f"Destination model target json {args.config} not found.")
    # add target.json to the path if it points to a directory
    if os.path.isdir(args.config):
        args.config = os.path.join(args.config, "target.json")

    # create a Repo object for the input directory
    input_repo = Repo.from_json(os.path.join(args.input, "target.json"))

    # create a Translator object and translate the input repository
    translator_cls = get_translator_cls(args.method, args.naive_llm)
    translator = translator_cls(input_repo,
                                args.output,
                                args.src_model,
                                args.dst_model,
                                args.output_id,
                                args.app_name,
                                args.naive_llm,
                                args.config)
    translator.translate(dry=args.dry, log_interactions=args.log_interactions,
                         hide_progress=args.hide_progress)



if __name__ == "__main__":
    main()
