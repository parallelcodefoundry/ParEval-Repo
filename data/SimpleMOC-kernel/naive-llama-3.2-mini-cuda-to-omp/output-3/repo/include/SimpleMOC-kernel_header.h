// Translated from SimpleMOC-kernel_header.h
// To be used with OpenMP offload execution model

#ifndef SIMPLEMOC_KERNEL_HEADER_H
#define SIMPLEMOC_KERNEL_HEADER_H

#include "cuda_runtime_api.h"

typedef struct {
    float dx;
    float maxVal;
    int N;
    float *values;
} Table;

typedef struct {
    int size;
    float *values;
} SourceArrays;

__device__ void __checkError(const char* file, int line) {
#ifdef CUDA_ERROR_CHECK
    cudaError_t err = cudaGetLastError();
    if (cudaSuccess != err) {
        fprintf(stderr, "checkError() failed at %s:%i : %s\n", file, line, cudaGetErrorString(err));
        exit(-1);
    }

    // More careful checking.
    err = cudaDeviceSynchronize();
    if (cudaSuccess != err) {
        fprintf(stderr, "checkError() with sync failed at %s:%i : %s\n", file, line, cudaGetErrorString(err));
        exit(-1);
    }
#endif
}

__global__ void kernel(float* values) {
    __checkError(__FILE__, __LINE__);
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < values.length) {
        // OpenMP offload implementation
        #pragma omp target(map(tofrom:values)) schedule(dynamic, 1)
        {
            values[index] = exp(-index);
        }
    }
}

Table buildExponentialTable() {
    __checkError(__FILE__, __LINE__);
    int N = 353;
    float maxVal = 10.0;
    float dx = maxVal / (float)N;

    Table table;
    table.values = (float*)malloc(N * sizeof(float));
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    for (int n = 0; n < N; n++) {
        float exponential = exp(-n * dx);
        table.values[2*n] = -exponential;
        table.values[2*n + 1] = 1 + (n * dx - 1) * exponential;
    }

    return table;
}

SourceArrays initializeDeviceSources(int size, SourceArrays* sources_h, SourceArrays* SA_d, Source* sources_h_ptr) {
    __checkError(__FILE__, __LINE__);
    cudaMalloc((void**) &SA_d->values, size * sizeof(float));
    cudaMemcpy(SA_d->values, sources_h->values, size * sizeof(float), cudaMemcpyHostToDevice);

    return *SA_d;
}

#endif // SIMPLEMOC_KERNEL_HEADER_H