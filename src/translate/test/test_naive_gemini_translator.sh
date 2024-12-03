for iter in {1..5}; do
    python3 ../translate.py --input ../../../targets/XSBench/cuda --output ../../../data/XSBench/naive-gemini-cuda-to-omp --output-id ${iter} --app-name xsbench --method naive --naive-llm gemini --src-model cuda --dst-model openmp-offload --force-overwrite --log-interactions
    sleep 15
done
