for iter in {0..0}; do
   python3 /Users/ishan/pssg/code-translation/src/SWE-agent/swe-agent-translator.py \
       --agent_model_name gpt-4o-mini \
       --per_instance_cost_limit 0.06 \
       --repo_path /Users/ishan/pssg/code-translation/targets/SimpleMOC-kernel/cuda \
       --deployment_image python \
       --problem_statement_path /Users/ishan/pssg/code-translation/targets/SimpleMOC-kernel/cuda/translation_task.md \
       --output_dir /Users/ishan/pssg/code-translation/data/SimpleMOC-kernel/SWE-agent-cuda-to-omp/output-$iter
done