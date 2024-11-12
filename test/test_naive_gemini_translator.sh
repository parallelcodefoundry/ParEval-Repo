for iter in {1..5}; do
    python3 ../src/translate.py --input ../targets/microXOR/cuda --output ../data/microXOR/naive-gemini-cuda-to-omp --output-id ${iter} --app-name microxor --method gemini --src-model cuda --dst-model openmp-offload
    sleep 15
done
