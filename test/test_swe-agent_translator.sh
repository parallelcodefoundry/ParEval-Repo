for iter in {0..4}; do
    python3 ../src/agent/swe-agent-translator.py --repo_path "/Users/ishan/tmp/microXOR_cuda_repo" --output_base_dir "/Users/ishan/pssg/code-translation/data/microXOR/SWE-agent-cuda-to-omp" --keys_cfg "/Users/ishan/pssg/SWE-agent/keys.cfg" --config_file "/Users/ishan/pssg/SWE-agent/config/default.yaml" --data_path "/Users/ishan/pssg/code-translation/targets/microXOR/translation_task.md" --model_name "gpt4omini" --per_instance_cost_limit 0.50 --input_directory "/Users/ishan/pssg/code-translation/targets/microXOR/cuda"
    sleep 30
done