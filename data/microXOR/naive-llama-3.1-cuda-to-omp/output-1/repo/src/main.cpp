#include <omp.h>
#include <stdio.h>

// Define a function to launch the kernel in parallel
void launch_kernel(int *input, int *output, size_t N) {
  // Create an array of block sizes to use for each thread block
  static const int blockSizes[] = {32, 16, 8, 4};

  #pragma omp parallel num_threads(omp_get_num_procs())
  {
    int tid = omp_get_thread_num();
    int nthreads = omp_get_num_threads();

    // Choose a block size from the array or use the number of threads
    int blockSize = (tid < sizeof(blockSizes) / sizeof(int)) ? blockSizes[tid] : nthreads;

    // Calculate the global thread ID and calculate the global block ID
    int iGlobal, jGlobal;
    #pragma omp master
    {
      iGlobal = (blockSize * tid + blockIdx.x * blockDim.x) % N;
      jGlobal = (blockSize * tid + blockIdx.y * blockDim.y) % N;
    }

    // Perform the computation in parallel for each block
    #pragma omp for schedule(static)
    for (size_t i = 0; i < N / blockSize; ++i) {
      int iStart = i * blockSize;
      int iEnd = (i + 1) * blockSize;

      // Launch a new kernel for this block
      #pragma offload target(omp_target_device) in(input[blockSize*iStart:(blockSize-1)+blockSize*iStart], output[iStart*N:(blockSize-1)+N*iStart]) \
                       in(N), out(iGlobal, jGlobal)
      {
        cellsXOR<<< omp_get_num_blocks(), blockSize >>> (input + iStart * N, output + iStart * N, N);
      }

      // Wait for the kernel to finish
      #pragma offload target(omp_target_device) wait(device:cellsXOR<<<omp_get_num_blocks(), blockSize>>>)
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <input_size> <block_size>\n", argv[0]);
    return 1;
  }

  int N = atoi(argv[1]);
  int blockSize = atoi(argv[2]);

  // Allocate input and output arrays on the host
  int *input, *output;
  input = (int *) malloc(N * N * sizeof(int));
  output = (int *) malloc(N * N * sizeof(int));

  // Initialize the input array with random values
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      input[i * N + j] = rand() % 2;
    }
  }

  // Launch the kernel in parallel using OpenMP
  launch_kernel(input, output, N);

  // Validate the output
  int count = 0;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      int input_count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) input_count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) input_count++;
      if (j > 0 && input[i*N + (j-1)] == 1) input_count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) input_count++;

      if (input_count == 1) {
        if (output[i * N + j] != 1) {
          printf("Validation failed at (%d, %d)\n", i, j);
          free(input); free(output);
          return 1;
        }
      } else {
        if (output[i * N + j] != 0) {
          printf("Validation failed at (%d, %d)\n", i, j);
          free(input); free(output);
          return 1;
        }
      }

      count++;
    }
  }

  printf("Validation passed.\n");
  free(input); free(output);

  return 0;
}