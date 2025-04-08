#!/bin/bash
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -t 3:00:00
#SBATCH -J translate_vllm
#SBATCH --gpus-per-node 4
#SBATCH --exclusive
#SBATCH --partition=gpuA100x4
#SBATCH --account=bega-delta-gpu
#SBATCH --constraint="scratch"

cd /u/jhdavis/code-translation/src/translate/test
bash translate_vllm_instance.sh restate qwq-32b-q8_0.gguf SimpleMOC-kernel 1 25 runs/restate-qwq-simplemoc-kernel.out delta
