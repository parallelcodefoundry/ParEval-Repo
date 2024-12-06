#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

void io(float *data, int size) {
    #pragma omp target map(out(data)) distribute
    {
        // perform some IO operations here
        for (int i = 0; i < size; i++) {
            data[i] = sin(i);
        }
    }
}

#ifdef __cplusplus
}
#endif

// OpenMP offload function declaration
void io_offload(float *data, int size) {
    #pragma omp target map(out(data)) distribute
    {
        // perform some IO operations here
        for (int i = 0; i < size; i++) {
            data[i] = sin(i);
        }
    } openmp;
}

// OpenMP offload function call
void io_offload_call(float *data, int size) {
    io_offload(data, size);
}

#ifdef __cplusplus
extern "C" {
#endif

// Function to print the results
void printResults() {
    float data[size];
    io_offload_call(data, size);
    for (int i = 0; i < size; i++) {
        printf("%f ", data[i]);
    }
    printf("\n");
}

#ifdef __cplusplus
}
#endif