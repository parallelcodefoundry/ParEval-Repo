#!/usr/bin/env python3
""" This script generates a list of jobs for the Delta system. It reads the job
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
#SBATCH --gpus-per-node 4
#SBATCH --exclusive
#SBATCH --partition=gpuA100x4
#SBATCH --account=bega-delta-gpu
#SBATCH --constraint="scratch"

cd /u/jhdavis/code-translation/src/translate/test
bash translate_vllm_instance.sh {job_config['method']} {job_config['model']} {job_config['app']} {job_config['start_idx']} {job_config['end_idx']} runs/{job_name}.out delta
"""

    # Create the job script file
    script_filename = f"{job_name}.slurm"
    with open(script_filename, 'w') as script_file:
        script_file.write(script)
    print(f"Generated job script: {script_filename}")
    return script_filename

def generate_batch_script(job_names):
    """Generate a batch script to enqueue all the jobs."""
    batch_script = "#!/bin/bash\n"
    for job_name in job_names:
        batch_script += f"sbatch {job_name}.slurm\n"

    # Create the batch script file
    batch_script_filename = "enqueue_jobs.sh"
    with open(batch_script_filename, 'w') as batch_file:
        batch_file.write(batch_script)
    print(f"Generated batch script: {batch_script_filename}")
    return batch_script_filename

if __name__ == "__main__":
    # Check if the input YAML file is provided
    if len(sys.argv) != 2:
        print("Usage: python delta-generate-jobs.py <config_file>")
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
