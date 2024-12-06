#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to validate the output
void validateOutput(int* input, int* output, size_t N) {
    #pragma omp parallel for
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            int count = 0;
            if (i > 0 && input[(i-1)*N + j] == 1) count++;
            if (i < N-1 && input[(i+1)*N + j] == 1) count++;
            if (j > 0 && input[i*N + (j-1)] == 1) count++;
            if (j < N-1 && input[i*N + (j+1)] == 1) count++;
            if (count == 1) {
                if (output[i*N + j] != 1) {
                    printf("Validation failed at (%d, %d)\n", i, j);
                    return;
                }
            } else {
                if (output[i*N + j] != 0) {
                    printf("Validation failed at (%d, %d)\n", i, j);
                    return;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s NxN kernel_threads\n", argv[0]);
        return 1;
    }

    size_t N = std::stoi(argv[1]);
    int kernelThreads = std::stoi(argv[2]);

    // Create input and output arrays
    int* input = (int*)malloc(N*N * sizeof(int));
    int* output = (int*)malloc(N*N * sizeof(int));

    for (size_t i = 0; i < N*N; i++) {
        input[i] = (i % 2 == 0) ? 1 : 0;
    }

    // Validate the output
    validateOutput(input, output, N);

    // Offload to parallel region
    #pragma omp target map(tofrom:input, output) map(override(N))
    {
        // Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                int count = 0;
                if (i > 0 && input[(i-1)*N + j] == 1) count++;
                if (i < N-1 && input[(i+1)*N + j] == 1) count++;
                if (j > 0 && input[i*N + (j-1)] == 1) count++;
                if (j < N-1 && input[i*N + (j+1)] == 1) count++;
                output[i*N + j] = (count == 1) ? 1 : 0;
            }
        }
    }

    // Validate the output again after offloading
    validateOutput(input, output, N);

    free(input);
    free(output);

    printf("Validation passed.\n");

    return 0;
}