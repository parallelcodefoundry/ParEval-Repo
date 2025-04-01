#!/bin/bash
get_usage () {
    echo "Usage: translate.sh method llm_name llm_backend [application] [outputid] [finid]"
    echo "method: the method to use (e.g., naive, restate)"
    echo "llm_name: the name of the LLM to use (e.g., gpt-4o-mini, gemini-2.0-flash)"
    echo "llm_backend: the backend to use (e.g., openai, gemini)"
    echo "application: the name of the application to translate"
    echo "             (e.g., microXOR, XSBench, SimpleMOC-kernel, llm.c)"
    echo "outputid: the integer output ID to use for the translation, requires application"
    echo "finid: providing this makes the script run a translation for each in outputid..finid"
}

run_translate () {
    method=$1
    llm_version=$2
    base_llm_version=$(basename -- "$llm_version")
    llm_name="${base_llm_version%.*}"
    if [[ $llm_version == *"gemini"* ]]; then
        llm_name=${llm_version}
    fi
    llm_backend=$3
    application=$4
    iter=$5
    python3 ../translate.py \
            --input ../../../targets/${application}/cuda \
            --config ../../../targets/${application}/openmp-offload \
            --output ../../../../code-translation-results/outputs/${application}/${method}-${llm_name}-cuda-to-omp \
            --output-id ${iter} \
            --app-name ${application} \
            --method ${method} \
            --${method}-backend ${llm_backend} \
            --${method}-llm-name ${llm_version} \
            --src-model cuda \
            --dst-model openmp-offload \
            --log-interactions \
            -f
    if [ $? -ne 0 ]; then
        echo "Failed at iteration ${iter}"
        exit 1
    fi
}

if [ "$#" -eq 3 ]; then
    method=$1
    llm_version=$2
    llm_backend=$3
    echo "Running all ${method} experiments for ${llm_version} (${llm_backend})"
    for application in "microXOR" "XSBench" "SimpleMOC-kernel" "llm.c"; do
        for iter in {1..5}; do
            run_translate ${method} ${llm_version} ${llm_backend} ${application} ${iter}
        done
    done
elif [ "$#" -eq 4 ]; then
    method=$1
    llm_version=$2
    llm_backend=$3
    application=$4
    echo "Running all ${method} experiments for ${llm_version} (${llm_backend}) and ${application}"
    for iter in {1..50}; do
        run_translate ${method} ${llm_version} ${llm_backend} ${application} ${iter}
    done
elif [ "$#" -eq 5 ]; then
    method=$1
    llm_version=$2
    llm_backend=$3
    application=$4
    iter=$5
    echo "Running one ${method} experiment for ${llm_version} (${llm_backend}) and ${application}, output ID ${iter}"
    run_translate ${method} ${llm_version} ${llm_backend} ${application} ${iter}
elif [ "$#" -eq 6 ]; then
    method=$1
    llm_version=$2
    llm_backend=$3
    application=$4
    fiter=$5
    liter=$6
    echo "Running ${fiter} thru ${liter} ${method} experiments for ${llm_version} (${llm_backend}) and ${application}"
    for iter in $(seq ${fiter} ${liter}); do
        run_translate ${method} ${llm_version} ${llm_backend} ${application} ${iter}
    done
else
    get_usage
    exit 1
fi
