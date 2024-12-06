for iter in {1..5}; do
    python3 ../translate.py --input ../../../targets/XSBench/cuda --output ../../../data/XSBench/naive-llama-3.2-mini-cuda-to-omp --output-id ${iter} --app-name XSBench --method naive --naive-llm llama-3.2 --src-model cuda --dst-model openmp-offload --log-interactions
    #sleep 15
done
