if [ "$#" -eq 1 ]; then
    llm_version=$1
    echo "Running all experiments for ${llm_version}"
    for application in "microXOR" "XSBench" "SimpleMOC-kernel" "llm.c"; do
        for iter in {1..5}; do
            python3 ../translate.py --input ../../../targets/${application}/cuda --config ../../../targets/${application}/openmp-offload --output ../../../data/${application}/naive-${llm_version}-cuda-to-omp --output-id ${iter} --app-name ${application} --method naive --naive-llm ${llm_version} --src-model cuda --dst-model openmp-offload --log-interactions
            sleep 15
        done
    done
elif [ "$#" -eq 2 ]; then
    llm_version=$1
    application=$2
    echo "Running all experiments for ${llm_version} and ${application}"
    for iter in {1..5}; do
        python3 ../translate.py --input ../../../targets/${application}/cuda --config ../../../targets/${application}/openmp-offload --output ../../../data/${application}/naive-${llm_version}-cuda-to-omp --output-id ${iter} --app-name ${application} --method naive --naive-llm ${llm_version} --src-model cuda --dst-model openmp-offload --log-interactions
        sleep 15
    done
else
    echo "Usage: naive_translate.sh [llm_version] [application]"
    exit 1
fi
