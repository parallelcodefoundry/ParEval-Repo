#!/bin/bash
#SBATCH -A m2404
#SBATCH -C gpu
#SBATCH -q shared
#SBATCH -t 1:00:00
#SBATCH -N 1
#SBATCH --ntasks-per-node=1
#SBATCH --gpus-per-task=1

module load python/3.11
cd /global/homes/j/jhdavis/llms4hpc/code-translation/src/drivers
./run-all.py -o perlmutter-omp-nanoxor-restate.json -f --scratch-dir scratch -m openmp-offload -a nanoxor -p restate -t ../../targets/ --log INFO --system-config ../../config/perlmutter-config.json /global/homes/j/jhdavis/llms4hpc/code-translation-results/outputs
