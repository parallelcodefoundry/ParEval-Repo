# ParEval-Repo

ParEval-Repo is a LLM benchmarking suite for repository-scale translation of HPC (parallel) codes,
partially derived from [ParEval](https://github.com/parallelcodefoundry/ParEval). Contributions
are welcome and encouraged.

For more information, refer to [our ICPP '25 paper](https://arxiv.org/abs/2506.20938).

The layout of the repository is as follows:

- `src/`: Implementations of each translation method and drivers for testing.
  - `src/translate`: Translation methods (naive, non-agentic, SWE-agent)
  - `src/drivers`: Drivers for building and testing translations
- `targets/`: Repositories that will be translated from one programming model to another. Further
  organized as `targets/<repo>/<programming model>/`.
- `config/`: Configuration json files to inform drivers on how to load dependencies and run on
  various systems.

## Installation

To use ParEval-Repo we require Python 3.11.13 or greater along with several libraries listed in
`requirements.txt`. You can install these using pip or the package manager of your choice:

```
pip install -r requirements.txt
```

## Usage

Using ParEval-Repo consists of two steps, translation (LLM inference) and testing with our drivers.
The main script for translation is found under `src/translate/translate.py` and the driver main
script is at `src/drivers/run-all.py`. Each provides detailed usage directions with the `--help`
command line flag.

## Translating a new repository

The repositories we provide for testing translation are listed under `targets/`. Each repo and model
combination has its own subdirectory containing a `repo/` folder with the repo contents (source
code, inputs, etc.) as well as a `target.json` that tells the driver and translation scripts how to
use the repo as well as other key metadata.

A `target.json` file is required for the testing drivers, and for translation we require a
`target.json` be provided for both the input repo and the output repo. This is because we tell the
LLMs what commands will be used to build and run the translated repo. `target.json` files for the
cases we test in the paper are provided under `targets` in the appropriate subdirectories.

## Citing ParEval-Repo

If you use ParEval-Repo, please cite our paper as follows:

```
Davis, J. H., Nichols, D., Khillan, I., & Bhatele, A. (2025). ParEval-Repo: A Benchmark Suite for Evaluating LLMs with Repository-level HPC Translation Tasks. arXiv preprint arXiv:2506.20938.
```