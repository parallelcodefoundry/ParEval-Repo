#!/bin/bash
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
