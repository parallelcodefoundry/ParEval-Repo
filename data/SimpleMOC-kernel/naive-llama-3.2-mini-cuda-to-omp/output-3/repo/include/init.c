#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 256

__global__
void init_cuda(const int* n, const float* seed, Source* sources_h) {
    // Get thread ID within a workgroup of size NUM_THREADS
    int tid = blockIdx.x * blockDim.x + threadIdx.tid;
    
    if (tid < n) {
        // Set seed for each thread
        for (int i = 0; i < 3; i++) {
            sources_h[tid].values[i] = seed[tid];
        }
    }

    __cudaCheckError(__FILE__, __LINE__);
}

void init_openmp(const int* n, const float* seed, Source* sources_h) {
    for (int tid = 0; tid < *n; tid++) {
        for (int i = 0; i < 3; i++) {
            sources_h[tid].values[i] = seed[tid];
        }
    }
}

Table buildExponentialTable() {
    // define table
    Table table;

    //float precision = 0.01;
    float maxVal = 10.0;	

    // compute number of arry values
    //int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * precision * 0.01 ) ) );
    int N = 353; 

    // compute spacing
    float dx = maxVal / (float) N;

    // store linear segment information (slope and y-intercept)
    for (int n = 0; n < N; n++) {
        // compute slope and y-intercept for ( 1 - exp(-x) )
        float exponential = exp( - n * dx );
        table.values[ 2*n ] = - exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
    }

    // assign data to table
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    return table;
}