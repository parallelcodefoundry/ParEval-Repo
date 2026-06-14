#!/bin/bash
#SBATCH -q regular
#SBATCH -A m5083
#SBATCH -t 02:00:00
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -c 16
#SBATCH -C "gpu&hbm80g"
#SBATCH --gpus-per-node 4
#SBATCH --array=0-35
#SBATCH -o translate-sweep-%A-%a.out
#SBATCH -e translate-sweep-%A-%a.out

set -euo pipefail

# Sweep configuration — edit here to change what the array tests.
# len(TRANSLATE_METHODS) * len(TRANSLATE_PAIRS) * len(TRANSLATE_APPS) must equal the --array bound
export TRANSLATE_APPS="nanoXOR microXORh microXOR XSBench SimpleMOC-kernel llm.c"
export TRANSLATE_PAIRS="cuda,openmp-offload cuda,kokkos openmp-threads,openmp-offload"
export TRANSLATE_METHODS="naive top-down-agentic"
export TRANSLATE_NUM=20

module load python/3.12 cudatoolkit/12.9 cudnn gcc-native/13.2
export CC=gcc CXX=g++

cd "${SLURM_SUBMIT_DIR}"
. .venv/bin/activate

. translate-runner.sh
run_translate_array_task
