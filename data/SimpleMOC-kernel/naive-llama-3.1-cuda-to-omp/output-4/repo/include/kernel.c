#include <omp.h>
#include <cuda_runtime_api.h>

#define CUDA_CHECK(error) \
  do { \
    cudaError_t err = (error); \
    if (err != cudaSuccess) { \
      printf("CUDA error: %s\n", cudaGetErrorString(err)); \
      exit(1); \
    } \
  } while (0)

#define HANDLE_ERROR(err) \
  do { \
    if ((err) != cudaSuccess) { \
      fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString((err))); \
      exit(EXIT_FAILURE); \
    } \
  } while (0)

#pragma offload target(mic, map_arg_kernel)
int main() {
  // initialize device
  int dev = omp_get_max_threads();
  CUDA_CHECK(cudaSetDevice(dev));

  // allocate memory on the device
  float *d_sources;
  cudaMalloc((void **)&d_sources, sizeof(Source) * numSources);

  // copy data from host to device
  Source *sources = (Source *)malloc(sizeof(Source) * numSources);
  for (int i = 0; i < numSources; ++i) {
    sources[i].sigT_id = i;
  }
  CUDA_CHECK(cudaMemcpy(d_sources, sources, sizeof(Source) * numSources, cudaMemcpyHostToDevice));

  // execute kernel
  #pragma omp parallel
  {
    int tid = omp_get_thread_num();
    int num_threads = omp_get_num_threads();

    // allocate memory for each thread
    float *d_sources_thread;
    cudaMalloc((void **)&d_sources_thread, sizeof(Source) * numSources);

    // copy data from device to thread-local memory
    CUDA_CHECK(cudaMemcpy(d_sources_thread, d_sources, sizeof(Source) * numSources, cudaMemcpyDeviceToDevice));

    // execute kernel function
    kernel_function(d_sources_thread, numSources);
  }

  // synchronize threads
  #pragma omp barrier

  // deallocate memory on the device
  cudaFree(d_sources);

  return 0;
}

void kernel_function(float *d_sources, int numSources) {
  // perform computations here
}