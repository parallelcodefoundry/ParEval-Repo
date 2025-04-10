#!/usr/bin/env python3
""" This script generates a list of jobs for the Zaratan system. It reads the job
    configuration from a YAML file and generates a Slurm script for each job, as
    well as batch script to enqueue all the generated jobs.
"""

import os
import sys
import subprocess
import yaml

def generate_job_script(job_name, job_config):
    """Generate a Slurm script for a given job."""
    script = f"""#!/bin/bash
#SBATCH --job-name={job_name}
#SBATCH --output={job_name}.o
#SBATCH --error={job_name}.e
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -t {job_config['time']}
#SBATCH -p gpu
#SBATCH --gpus=a100:1

cd /home/jhdavis/llms4hpc/code-translation/src/drivers
. ~/spack/share/spack/setup-env.sh
source /scratch/zt1/project/bhatele-lab/user/jhdavis/vllm_venv/bin/activate
python3 run-all.py -y -s \
    --scratch-dir $SCRATCH/trns \
    -t ../../targets/ \
    --log DEBUG \
    --system-config ../../config/zaratan-config.json \
    -p {job_config['method']} \
    -a {job_config['app']} \
    -m {job_config['dst_model']} \
    -o zaratan-{job_name}.json \
    {'--skip-build-swap' if 'skip_build_swap' in job_config and job_config['skip_build_swap'] != 'no' else ''} \
    /home/jhdavis/llms4hpc/code-translation-results/outputs &> runs/{job_name}.out
if [ $? -eq 0 ]; then
  echo "Completed driver run."
fi
"""

    # Create the job script file
    script_filename = f"gen_{job_name}.slurm"
    with open(script_filename, 'w') as script_file:
        script_file.write(script)
    print(f"Generated job script: {script_filename}")
    return script_filename


def generate_batch_script(job_names):
    """Generate a batch script to enqueue all the jobs."""
    batch_script = "#!/bin/bash\n"
    for job_name in job_names:
        batch_script += f"sbatch gen_{job_name}.slurm\n"

    # Create the batch script file
    batch_script_filename = "enqueue_jobs.sh"
    with open(batch_script_filename, 'w') as batch_file:
        batch_file.write(batch_script)
    print(f"Generated batch script: {batch_script_filename}")
    return batch_script_filename


if __name__ == "__main__":
    # Check if the input YAML file is provided
    if len(sys.argv) != 2:
        print("Usage: python zaratan-generate-jobs.py <config_file>")
        sys.exit(1)

    config_file = sys.argv[1]

    # Load the job configuration from the YAML file
    with open(config_file, 'r') as file:
        job_configs = yaml.safe_load(file)

    job_names = []
    for job_name, job_config in job_configs.items():
        script_filename = generate_job_script(job_name, job_config)
        job_names.append(job_name)

    # Generate the batch script to enqueue all jobs
    generate_batch_script(job_names)
