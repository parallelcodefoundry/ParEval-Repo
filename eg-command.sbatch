#!/bin/bash
#SBATCH -q regular
#SBATCH -A m5083
#SBATCH -t 02:00:00
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -c 16
#SBATCH -C "gpu&hbm80g"
#SBATCH --gpus-per-node 4
#SBATCH -o eg-translate-%j.out
#SBATCH -e eg-translate-%j.out

set -euo pipefail

module load python/3.12 cudatoolkit/12.9 cudnn gcc-native/13.2
export CC=gcc CXX=g++
. .venv/bin/activate

python src/translate/translate.py \
       -i targets/nanoXOR/cuda/ \
       -o ../restate-results/ \
       -c targets/nanoXOR/openmp-offload/ \
       --method naive \
       --src-model cuda \
       --dst-mode openmp-offload \
       -n 1 \
       --output-id 0 \
       --app-name nanoXOR \
       --vllm-environment \
       ../serve/.venv/ \
       --vllm-yaml-config \
       config/perlmutter-vllm-oss.yaml \
       --naive-backend vllm \
       --naive-llm-name openai/gpt-oss-120b
