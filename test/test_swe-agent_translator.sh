for iter in {5..5}; do
   python3 /Users/ishan/pssg/code-translation/src/SWE-agent/translation_task_generator.py \
       --target_json /Users/ishan/pssg/code-translation/targets/SimpleMOC-kernel/cuda/target.json \
       --output_file /Users/ishan/pssg/code-translation/targets/SimpleMOC-kernel/cuda/translation_task.md

   python3 /Users/ishan/pssg/code-translation/src/SWE-agent/swe-agent-translator.py \
       --agent_model_name gpt-4o-mini \
       --per_instance_cost_limit 0.06 \
       --repo_path /Users/ishan/pssg/code-translation/targets/SimpleMOC-kernel/cuda \
       --deployment_image python \
       --problem_statement_path /Users/ishan/pssg/code-translation/targets/SimpleMOC-kernel/cuda/translation_task.md \
       --output_dir /Users/ishan/pssg/code-translation-results/SimpleMOC-kernel/SWE-agent-cuda-to-omp/output-$iter
done