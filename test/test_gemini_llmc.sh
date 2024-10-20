for iter in {0..4}; do
    python3 ../src/translate.py --input ../targets/llm.c/include --output ../data/llm.c/naive-gemini-cuda-to-omp/output-${iter} --method gemini --src-model cuda --dst-model openmp-offload
    sleep 15
done