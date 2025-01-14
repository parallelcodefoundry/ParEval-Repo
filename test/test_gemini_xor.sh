for iter in {1..5}; do
    python3 src/translate/translate.py \
        --input targets/microXOR/cuda \
        --output data/microXOR/agent-gpt4o-mini-cuda-to-omp \
        --output-id ${iter} \
        --app-name microxor \
        --method agent \
        --agent-backend openai \
        --agent-llm gpt-4o-mini \
        --src-model cuda \
        --dst-model openmp-offload

    if [ $? -ne 0 ]; then
        echo "Failed at iteration ${iter}"
        exit 1
    fi
    
    sleep 15
done
