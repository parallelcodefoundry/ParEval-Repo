// kernel.cu OpenMP Offload version

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 353

void offload_kernel(float *values, float dx, int maxVal) {
    // compute number of arry values
    int N = (int) (maxVal * sqrt(1.0 / (8.0 * dx)));

    // assign data to local array for computation
    float local_values[N];
    #pragma omp parallel for reduction(+:local_values)
    for (int n = 0; n < N; ++n) {
        float exponential = exp(-n*dx);
        values[2*n] = -exponential;
        values[2*n+1] = 1 + (n*dx-1)*exponential;
        local_values[n] = values[2*n];
    }

    // copy results to global array
    #pragma omp parallel for
    for (int n = 0; n < N; ++n) {
        values[2*n] = local_values[n];
    }
}

// device function for OpenMP Offload
__global__
void kernel(float *values, float dx, int maxVal) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        offload_kernel(values + idx*2, dx, maxVal);
    }
}

// compute number of threads
int num_threads(int N) {
    return (N == 1) ? 256 : (N > 10000) ? (N - 1) * 16 + 64 : 256;
}