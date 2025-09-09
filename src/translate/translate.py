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
import shutil
import tarfile

# local imports
from repo import Repo
from translator import Translator
from naive.naive_translator import NaiveTranslator
from restate.top_down_agent import TopDownAgentTranslator
from swe_agent.swe_agent_translator import SWEAgentTranslator

def get_args():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("-i", "--input", type=str, required=True, help="Path to the input source code repository.")
    parser.add_argument("-o", "--output", type=str, required=True, help="Path to the output source code repository.")
    parser.add_argument("-c", "--config", type=str, required=True, help="Path to translation destination model configuration file containing prompt fill-ins.")
    parser.add_argument("-f", "--force-overwrite", action="store_true", help="Force overwrite of existing output directory.")
    parser.add_argument("--method", choices=["naive", "restate", "swe-agent"], required=True, help="The translation method to use.")
    parser.add_argument("--src-model", type=str, required=True, help="The source execution model.")
    parser.add_argument("--dst-model", type=str, required=True, help="The destination execution model.")
    parser.add_argument("--output-id", type=int, required=True, help="The integer ID of the output, used to count repeat instances of the same translation configuration.")
    parser.add_argument("-n", "--num-translations", type=int, default=1, help="The number of translations to generate. Currently only supported for OpenAI and vLLM with naive, will write to directories numbered from output-id through output-id + num-translations - 1.")
    parser.add_argument("--app-name", type=str, help="The name of the application being translated.")
    parser.add_argument("--dry", "-d", action="store_true", help="Dry run the translation.")
    parser.add_argument("--log-interactions", action="store_true", help="Log the raw LLM outputs to a text file.")
    parser.add_argument("--hide-progress", action="store_true", help="Hide the progress bar.")
    parser.add_argument("--tar-outputs", action="store_true", help="Create a tarball of the output directories.")

    # subgroup of arguments for the naive translation method
    naive_args = parser.add_argument_group("naive translation method")
    NaiveTranslator.add_args(naive_args)

    # subgroup for top-down agent
    agent_args = parser.add_argument_group("top-down agent")
    TopDownAgentTranslator.add_args(agent_args)

    # subgroup for SWE-agent translation method
    swe_agent_args = parser.add_argument_group("SWE-agent translation")
    SWEAgentTranslator.add_args(swe_agent_args)

    return parser.parse_args()

def get_translator_cls(method: str):
    if method == "naive":
        return NaiveTranslator
    if method == "restate":
        return TopDownAgentTranslator
    if method == "swe-agent":
        return SWEAgentTranslator
    raise ValueError(f"Translation method {method} not recognized.")


def main():
    args = get_args()

    # check if the input directory exists
    if not os.path.exists(args.input):
        raise FileNotFoundError(f"Input directory {args.input} not found.")

    # check if the output directories exist and are empty
    output_dirs = [os.path.join(args.output, "output-" + str(i)) \
                   for i in range(args.output_id, args.output_id + args.num_translations)]
    if os.path.exists(args.output):
        if not args.force_overwrite:
            for d in output_dirs:
                if os.path.exists(d):
                    raise FileExistsError(f"Output directory {d} already exists. " +
                                          "Provide --force-overwrite to overwrite.")
    else:
        os.makedirs(args.output, exist_ok=True)

    # check that the dest model target json for prompt config exists
    if not os.path.exists(args.config):
        raise FileNotFoundError(f"Destination model target json {args.config} not found.")
    # add target.json to the path if it points to a directory
    if os.path.isdir(args.config):
        args.config = os.path.join(args.config, "target.json")
    dst_config = json.load(open(args.config, "r"))

    # create a Repo object for the input directory
    input_repo = Repo.from_json(os.path.join(args.input, "target.json"))

    # create a Translator object and translate the input repository
    translator_cls = get_translator_cls(args.method)
    translator_args = translator_cls.parse_args(args)
    translator = translator_cls(
        input_repo=input_repo,
        output_repos=output_dirs,
        src_model=args.src_model,
        dst_model=args.dst_model,
        dst_config=dst_config,
        log_interactions=args.log_interactions,
        dry=args.dry,
        hide_progress=args.hide_progress,
        **translator_args
    )
    translator.translate()

    def create_tarball(output_dir):
        """ Create a tarball of the output directory. """
        tarball_name = f"output-{output_dir.split('-')[-1]}.tar.gz"
        tarball_path = os.path.join(args.output, tarball_name)

        with tarfile.open(tarball_path, "w:gz") as tar:
            arcname = os.path.basename(output_dir)
            tar.add(output_dir, arcname=arcname)

        # deletes output directory after creating tarball
        shutil.rmtree(output_dir)

    # create tarballs for each output directory
    if args.tar_outputs:
        for i in range(args.output_id, args.output_id + args.num_translations):
            output_dir = os.path.join(args.output, "output-" + str(i))
            if os.path.exists(output_dir):
                create_tarball(output_dir)
            else:
                print(f"Output directory {output_dir} does not exist. Skipping tarball creation.")

    # translator implements GeneratorMixin, then call print_stats
    if hasattr(translator, "print_stats"):
        translator.print_stats()


if __name__ == "__main__":
    main()
