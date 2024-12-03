for iter in {1..5}; do
    python3 ../translate.py --input ../../../targets/microXOR/cuda --output ../../../data/microXOR/naive-gemini-cuda-to-omp --output-id ${iter} --app-name microxor --method naive --naive-llm gemini --src-model cuda --dst-model openmp-offload --dry
    sleep 15
done
