for iter in {0..4}; do
    python3 ../src/translate.py --input ../targets/SimpleMOC-kernel/cuda/include --output ../data/SimpleMOC-kernel/naive-gemini-cuda-to-omp --output-id ${iter} --app-name simplemoc-kernel --method gemini --src-model cuda --dst-model openmp-offload
    sleep 15
done
