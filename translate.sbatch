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

app_names=( "nanoXOR" "microXORh" "microXOR" "XSBench" "SimpleMOC-kernel" "llm.c")
translate_pairs=("cuda,openmp-offload" "cuda,kokkos" "openmp-threads,openmp-offload")
methods=("naive" "top-down-agentic")
num_translations=20


for method in "${methods[@]}"; do
    for translate_pair in "${translate_pairs[@]}"; do
        src_model=$(echo "$translate_pair" | cut -d',' -f1)
        dst_model=$(echo "$translate_pair" | cut -d',' -f2)
        for app_name in "${app_names[@]}"; do
            for i in $(seq 0 $((num_translations - 1))); do
                python src/translate/translate.py \
                    -i "targets/${app_name}/${src_model}/" \
                    -o ../restate-results/ \
                    -c "targets/${app_name}/${dst_model}/" \
                    --method "${method}" \
                    --src-model "${src_model}" \
                    --dst-mode "${dst_model}" \
                    -n 1 \
                    --output-id "${i}" \
                    --app-name "${app_name}" \
                    --vllm-environment ../serve/.venv/ \
                    --vllm-yaml-config config/perlmutter-vllm-oss.yaml \
                    --naive-backend vllm \
                    --naive-llm-name openai/gpt-oss-120b
            done
        done
    done
done
