for iter in {1..5}; do
    python3 ../translate.py --input ../../../targets/SimpleMOC-kernel/cuda --output ../../../data/SimpleMOC-kernel/naive-gemini-cuda-to-omp --output-id ${iter} --app-name SimpleMOC-kernel --method naive --naive-llm gemini --src-model cuda --dst-model openmp-offload --force-overwrite --log-interactions
    sleep 15
done
