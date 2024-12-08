Here is the translation of the provided code into an OpenMP-offload executable model:

```c
// Offload directive to indicate that this kernel should be executed on a device (GPU)
#pragma offload target(mic) in(sources)

int main() {
  // Initialize the random number generator for reproducibility
  srand(0);

  int source_3D_regions = 1024;
  Input *input = malloc(sizeof(Input));
  Source_Arrays *source_arrays_host = malloc(sizeof(Source_Arrays));

  // Initialize host-side data structures and memory
  initializeHostData(input, source_arrays_host);
  sources = initialize_sources(source_arrays_host);

  // Offload the execution of this kernel to a device (GPU)
#pragma offload target(mic) in(sources) out(result)

  kernel_offload(sources, input, source_arrays_host);

  return 0;
}

void kernel_offload(Source *sources, Input *input, Source_Arrays *source_arrays_host) {
  // Offload directive to indicate that this kernel should be executed on a device (GPU)
#pragma offload target(mic) in(sources) out(result)

  // Initialize the host-side data structures and memory
  initializeDeviceData(input, source_arrays_host);

  // Allocate memory for device-side data structures
  Source_Arrays *source_arrays_device = malloc(sizeof(Source_Arrays));
  cudaMalloc((void **)&source_arrays_device->fine_source_arr, input->num_sources * sizeof(float));

  // Copy host-side data to device-side data structures
  cudaMemcpy(source_arrays_device->fine_source_arr, source_arrays_host->fine_source_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);

  // Perform kernel operations on the device (GPU)
  kernel_kernel(sources, input, source_arrays_device);

  return;
}

void kernel_kernel(Source *sources, Input *input, Source_Arrays *source_arrays_device) {
  // Offload directive to indicate that this kernel should be executed on a device (GPU)
#pragma offload target(mic) in(sources) out(result)

  // Perform kernel operations on the device (GPU)
  kernel_cuda_kernel(sources, input, source_arrays_device);

  return;
}

void initializeHostData(Input *input, Source_Arrays *source_arrays_host) {
  // Initialize host-side data structures and memory
  srand(0);
  int num_sources = 1024;

  // Allocate memory for host-side arrays
  source_arrays_host->fine_source_arr = malloc(num_sources * sizeof(float));
  source_arrays_host->fine_flux_arr = malloc(num_sources * sizeof(float));
  source_arrays_host->sigT_arr = malloc(num_sources * sizeof(float));

  // Initialize host-side data to random values
  for (int i = 0; i < num_sources; i++) {
    source_arrays_host->fine_source_arr[i] = (float)rand() / RAND_MAX;
    source_arrays_host->fine_flux_arr[i] = (float)rand() / RAND_MAX;
    source_arrays_host->sigT_arr[i] = (float)rand() / RAND_MAX;
  }

  return;
}

void initializeDeviceData(Input *input, Source_Arrays *source_arrays_host) {
  // Allocate memory for device-side arrays
  cudaMalloc((void **)&source_arrays_host->fine_source_arr, input->num_sources * sizeof(float));
  cudaMalloc((void **)&source_arrays_host->fine_flux_arr, input->num_sources * sizeof(float));
  cudaMalloc((void **)&source_arrays_host->sigT_arr, input->num_sources * sizeof(float));

  // Copy host-side data to device-side arrays
  cudaMemcpy(source_arrays_host->fine_source_arr, source_arrays_host->fine_source_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(source_arrays_host->fine_flux_arr, source_arrays_host->fine_flux_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(source_arrays_host->sigT_arr, source_arrays_host->sigT_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);

  return;
}

void initializeDeviceSources(Source *sources, Input *input, Source_Arrays *source_arrays_device) {
  // Allocate memory for device-side arrays
  cudaMalloc((void **)&source_arrays_device->fine_source_arr, input->num_sources * sizeof(float));
  cudaMalloc((void **)&source_arrays_device->fine_flux_arr, input->num_sources * sizeof(float));
  cudaMalloc((void **)&source_arrays_device->sigT_arr, input->num_sources * sizeof(float));

  // Copy host-side data to device-side arrays
  cudaMemcpy(source_arrays_device->fine_source_arr, source_arrays_host->fine_source_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(source_arrays_device->fine_flux_arr, source_arrays_host->fine_flux_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(source_arrays_device->sigT_arr, source_arrays_host->sigT_arr, input->num_sources * sizeof(float), cudaMemcpyHostToDevice);

  return;
}

void kernel_cuda_kernel(Source *sources, Input *input, Source_Arrays *source_arrays_device) {
  // Perform kernel operations on the device (GPU)
  cuda_kernel(sources, input, source_arrays_device);
}
```

This code is an example of how OpenMP-offload directives can be used to offload execution of kernels to a device (GPU). The code assumes that you have already set up your environment with OpenMP and offloading capabilities. Note that this is just one possible way to translate the original code into an OpenMP-offload executable model, and there may be other ways to do so depending on your specific needs.

Also note that some parts of the code (e.g., `kernel_cuda_kernel`) are not explicitly defined in the provided example, as they would depend on the actual implementation details of the kernel being offloaded. You will need to provide the implementation for these functions according to your requirements.

To compile this code using GCC or Clang with OpenMP support:

```bash
$ gcc -g -o example example.c -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp
```

Or, for a MIC architecture (Knights Corner) target:

```bash
$ icc -g -o example example.c -qoffload-architecture=mic:rocm
```