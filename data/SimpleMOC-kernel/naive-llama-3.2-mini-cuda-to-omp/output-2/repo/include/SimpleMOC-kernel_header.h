#ifndef SIMPLEMOC_KERNEL_HEADER_H
#define SIMPLEMOC_KERNEL_HEADER_H

#include <omp.h>
#include <cuda_runtime.h>

typedef struct {
    float dx;
    float maxVal;
    int N;
    float values[2*N];
} Table;

typedef struct {
    Source *sources;
    Source_Arrays *SA_d;
    Source_Arrays *SA_h;
    cudaStream_t stream;
} Kernel;

extern void kernel_main(Kernel *kernel);
extern Table buildExponentialTable(void);

#endif // SIMPLEMOC_KERNEL_HEADER_H