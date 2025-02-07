get_usage () {
    echo "Usage: restate_translate.sh llm_name llm_backend [application] [outputid]"
    echo "llm_name: the name of the LLM to use (e.g., gpt-4o-mini, gemini-1.5-flash)"
    echo "llm_backend: the backend to use (e.g., openai, gemini)"
    echo "application: the name of the application to translate"
    echo "             (e.g., microXOR, XSBench, SimpleMOC-kernel, llm.c)"
    echo "outputid: the integer output ID to use for the translation, requires application"
}

run_restate () {
    llm_name=$1
    llm_backend=$2
    application=$3
    iter=$4
    python3 ../translate.py \
            --input ../../../targets/${application}/cuda \
            --config ../../../targets/${application}/openmp-offload \
            --output ../../../../code-translation-results/${application}/restate-${llm_name}-cuda-to-omp \
            --output-id ${iter} \
            --app-name ${application} \
            --method agent \
            --agent-backend ${llm_backend} \
            --agent-llm ${llm_name} \
            --src-model cuda \
            --dst-model openmp-offload \
            --log-interactions \
            -f
    if [ $? -ne 0 ]; then
        echo "Failed at iteration ${iter}"
        exit 1
    fi
}

if [ "$#" -eq 2 ]; then
    llm_name=$1
    llm_backend=$2
    echo "Running all Restate experiments for ${llm_name} (${llm_backend})"
    for application in "microXOR" "XSBench" "SimpleMOC-kernel" "llm.c"; do
        for iter in {1..5}; do
            run_restate ${llm_name} ${llm_backend} ${application} ${iter}
        done
    done
elif [ "$#" -eq 3 ]; then
    llm_name=$1
    llm_backend=$2
    application=$3
    echo "Running all Restate experiments for ${llm_name} (${llm_backend}) and ${application}"
    for iter in {1..5}; do
        run_restate ${llm_name} ${llm_backend} ${application} ${iter}
    done
elif [ "$#" -eq 4 ]; then
    llm_name=$1
    llm_backend=$2
    application=$3
    iter=$4
    echo "Running one Restate experiment for ${llm_name} (${llm_backend}) and ${application}, output ID ${iter}"
    run_restate ${llm_name} ${llm_backend} ${application} ${iter}
else
    get_usage
    exit 1
fi
