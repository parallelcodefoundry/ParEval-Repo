for iter in {1..5}; do
    python3 src/translate/translate.py \
        --input targets/microXOR/cuda \
        --output data/microXOR/agent-gemini-cuda-to-omp \
        --output-id ${iter} \
        --app-name microxor \
        --method agent \
        --agent-backend gemini \
        --agent-llm gemini-1.5-flash \
        --src-model cuda \
        --dst-model openmp-offload \
        -f \
        --log-interactions

    if [ $? -ne 0 ]; then
        echo "Failed at iteration ${iter}"
        exit 1
    fi

    sleep 15
done
