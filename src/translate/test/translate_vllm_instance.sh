#!/bin/bash
get_usage () {
    echo "Usage: translate_vllm_instance.sh method llm_name application firstid lastid outpath"
    echo "method: the method to use (e.g., naive, restate)"
    echo "llm_name: the name of the LLM to use (e.g., Llama-3.3-70b, QwQ-32b)"
    echo "application: the name of the application to translate"
    echo "             (e.g., microXOR, XSBench, SimpleMOC-kernel, llm.c)"
    echo "fisrtid: integer ID for the first iteration to execute"
    echo "lastid: integer ID for the last iteration to execute"
    echo "outpath: file path to write all output for the run"
}

run_vllm_translation () {
    METHOD=$1
    LLM_NAME=$2
    APPLICATION=$3
    FITER=$4
    LITER=$5
    llm_path=$LLM_NAME

    # If model is already downloaded, then copy to /dev/shm/
    if [[ $LLM_NAME == *"gguf"* ]]; then
        llm_path=/dev/shm/$LLM_NAME
        echo -n "Copying model to /dev/shm/... "
        cp $SCRATCH/$LLM_NAME /dev/shm/
        echo "Done"
    fi

    # Activate vLLM environment
    source /home/jhdavis/venvs/2024-04-29-vllm/bin/activate

    # Set environment variables
    export HTTP_PROXY="http://proxy.alcf.anl.gov:3128"
    export HTTPS_PROXY="http://proxy.alcf.anl.gov:3128"
    export LD_LIBRARY_PATH=$HOME/venvs/2024-04-29-vllm/lib64/python3.11/site-packages/nvidia/nvjitlink/lib:$LD_LIBRARY_PATH
    export TMPDIR=/tmp/vllm && mkdir -p $TMPDIR
    export VLLM_HOST_IP=127.0.0.1
    export NO_PROXY=127.0.0.1,localhost
    export CUDA_VISIBLE_DEVICES=0,1,2,3

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

    # Do translation
    echo "Running translation $METHOD $llm_path vllm $APPLICATION $FITER $LITER... "
    cd /home/jhdavis/llms4hpc/code-translation/src/translate/test
    module use /soft/modulefiles
    module load llvm cudatoolkit-standalone
    bash translate.sh $METHOD $llm_path vllm $APPLICATION $FITER $LITER
    echo "Done"
}

stamp () { echo "[$$ | $(date '+%Y-%m-%d %H:%M:%S')]" }

if [ "$#" -eq 6 ]; then
    echo "$(stamp) Starting vLLM translation instance, writing to $6"
    run_vllm_translation "$1" "$2" "$3" "$4" "$5" &> $6
    echo "$(stamp) Finished vLLM translation instance, written to $6"
    exit
else
    get_usage
    exit 1
fi
