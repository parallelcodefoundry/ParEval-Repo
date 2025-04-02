#!/bin/bash
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -t 5:00
#SBATCH -J test_vllm
#SBATCH -p gpu
#SBATCH --gres=gpu:a100:4

source ~/vllm_venv/bin/activate

LLM_NAME="meta-llama/Llama-3.1-7B-Instruct"

# Start vLLM server
echo -n "Launching vLLM server process... "
unset vllm_flags
if [[ $LLM_NAME == *"Llama-3.3-70B"* ]]; then
    vllm_flags="--tokenizer meta-llama/Llama-3.3-70B-Instruct"
elif [[ $LLM_NAME == *"QwQ"* ]]; then
    vllm_flags="--enable-reasoning --reasoning-parser deepseek_r1"
fi
vllm serve $llm_path \
     --host 127.0.0.1 \
     --port 8000 \
     --api-key token_abc123 \
     --tensor-parallel-size 4 \
     --gpu-memory-utilization 0.95 \
     --enforce-eager \
     --enable-prefix-caching \
     $vllm_flags &
echo "Done"

VLLM_PID=$!  # Capture the PID of the server process

# Wait for server to start
echo "Waiting for vLLM server to become available... "
until curl -H "Authorization: Bearer token_abc123" -s http://127.0.0.1:8000/v1/models | grep -q "object"; do
    sleep 5
done
echo "Done"

# Check memory utlization across devices
nvidia-smi

# Exit
exit
