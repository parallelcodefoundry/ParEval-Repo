for iter in {0..0}; do
    python3 /global/homes/i/ikhillan/code-translation/src/agent/swe-agent-translator.py \
        --repo_path "/global/homes/i/ikhillan/tmp/microXOR_cuda_repo" \
        --output_base_dir "/global/homes/i/ikhillan/code-translation/data/microXOR/SWE-agent-cuda-to-omp" \
        --keys_cfg "/global/homes/i/ikhillan/SWE-agent/keys.cfg" \
        --config_file "/global/homes/i/ikhillan/SWE-agent/config/default.yaml" \
        --data_path "/global/homes/i/ikhillan/code-translation/targets/microXOR/translation_task.md" \
        --model_name "vllm:llama3.3" \
        --host_url "http://localhost:8000" \
        --input_directory "/global/homes/i/ikhillan/code-translation/targets/microXOR/cuda"

    python3 /global/homes/i/ikhillan/code-translation/src/agent/util/clean_ansi.py \
        --input_file "/global/homes/i/ikhillan/code-translation/data/microXOR/SWE-agent-cuda-to-omp/output-0/output-0-swe-agent-comments.txt" \
        --output_file "/global/homes/i/ikhillan/code-translation/data/microXOR/SWE-agent-cuda-to-omp/output-0/cleaned-comments.txt"
done