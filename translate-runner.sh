#!/usr/bin/env bash
# Runner for translate-sweep.sbatch job array.
# Each array task handles one (method, translate_pair, app) combination,
# running num_translations=20 translate.py calls against a shared vLLM server.
#
# Source this from translate-sweep.sbatch, then call run_translate_array_task.
# Requires SLURM_ARRAY_TASK_ID to be set (provided automatically by Slurm).

set -euo pipefail

run_translate_array_task() {
    local task_id="${SLURM_ARRAY_TASK_ID:-0}"

    # Task index decomposition — lists are defined in translate-sweep.sbatch.
    local -a app_names
    local -a translate_pairs
    local -a methods
    read -ra app_names     <<< "${TRANSLATE_APPS:?TRANSLATE_APPS not set}"
    read -ra translate_pairs <<< "${TRANSLATE_PAIRS:?TRANSLATE_PAIRS not set}"
    read -ra methods       <<< "${TRANSLATE_METHODS:?TRANSLATE_METHODS not set}"
    local n_apps=${#app_names[@]}
    local n_pairs=${#translate_pairs[@]}
    local num_translations="${TRANSLATE_NUM:?TRANSLATE_NUM not set}"

    local method_idx=$(( task_id / (n_pairs * n_apps) ))
    local pair_idx=$(( (task_id / n_apps) % n_pairs ))
    local app_idx=$(( task_id % n_apps ))

    local method="${methods[$method_idx]}"
    local translate_pair="${translate_pairs[$pair_idx]}"
    local app_name="${app_names[$app_idx]}"
    local src_model
    local dst_model
    src_model=$(echo "$translate_pair" | cut -d',' -f1)
    dst_model=$(echo "$translate_pair" | cut -d',' -f2)

    echo "Task ${task_id}: method=${method} src=${src_model} dst=${dst_model} app=${app_name}"

    # Per-task isolated cache dirs in node-local memory (/dev/shm).
    # Each array task gets its own slot so concurrent tasks don't collide.
    local lmem_cache="/dev/shm/${USER}/.cache"
    mkdir -p "$lmem_cache"
    local slot="${SLURM_ARRAY_TASK_ID:-0}"
    export TORCHINDUCTOR_CACHE_DIR="${lmem_cache}/torchinductor_${SLURM_JOB_ID}_${slot}"
    export VLLM_CACHE_ROOT="${lmem_cache}/vllm_${SLURM_JOB_ID}_${slot}"

    # A keepalive ID unique to this array task causes GeneratorMixin to write a
    # PID file for the vLLM server so it persists across all num_translations calls.
    local keepalive_id="${SLURM_JOB_ID}_${slot}"

    for i in $(seq 0 $((num_translations - 1))); do
        python src/translate/translate.py \
            -i "targets/${app_name}/${src_model}/" \
            -o ../restate-results/ \
            -c "targets/${app_name}/${dst_model}/" \
            --method "${method}" \
            --src-model "${src_model}" \
            --dst-model "${dst_model}" \
            -n 1 \
            --output-id "${i}" \
            --app-name "${app_name}" \
            --vllm-environment ../serve/.venv/ \
            --vllm-yaml-config config/perlmutter-vllm-oss.yaml \
            --vllm-keepalive-id "${keepalive_id}" \
            --naive-backend vllm \
            --naive-llm-name openai/gpt-oss-120b
    done
}
