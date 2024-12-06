#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 353 // table size

// table structure
typedef struct {
    float dx, maxVal;
    int N;
    float values[2*N];
} Table;

// function to build exponential table
Table buildExponentialTable(void) {
    // define table
    Table table;

    //float precision = 0.01;
    float maxVal = 10.0;	// compute number of arry values
    int N = (int) (maxVal * sqrt(1.0 / (8.0 * 0.01))); // compute spacing
    float dx = maxVal / (float)N;

    // store linear segment information (slope and y-intercept)
    for(int n = 0; n < N; n++) {
        // compute slope and y-intercept for ( 1 - exp(-x) )
        float exponential = exp(-n * dx);
        table.values[2*n] = -exponential;
        table.values[2*n + 1] = 1 + (n * dx - 1) * exponential;
    }

    // assign data to table
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    return table;
}

// kernel function for offloading
void kernelOffload(Table* table, int numThreadsPerBlock, __global__ float *output) {
    __shared__ float temp[2*N];
    int id = blockIdx.x * blockDim.x + threadIdx.x;

    // compute values in parallel using OpenMP
    #pragma omp parallel for
    for(int n = 0; n < table->N; n++) {
        float exponential = exp(-n * table->dx);
        temp[2*n] = -exponential;
        temp[2*n + 1] = 1 + (n * table->dx - 1) * exponential;

        // compute output using OpenMP
        #pragma omp critical
        {
            output[id] = temp[2*n];
        }
    }

    __syncthreads();

    // wait for all threads to finish
    #pragma omp parallel for
    for(int n = 0; n < table->N; n++) {
        float exponential = exp(-n * table->dx);
        output[id] = temp[2*n];
    }
}

int main() {
    Table table = buildExponentialTable();

    // allocate device memory
    __device__ float *outputD;
    cudaMalloc((void **)&outputD, table.N * sizeof(float));

    // launch kernel
    int numThreadsPerBlock = 256;
    int numBlocks = (table.N + numThreadsPerBlock - 1) / numThreadsPerBlock;
    kernelOffload(&table, numThreadsPerBlock, outputD);
    cudaDeviceSynchronize();

    // copy output to host memory
    __device__ float *outputH;
    cudaMallocHost((void **)&outputH, table.N * sizeof(float));
    cudaMemcpy(outputH, outputD, table.N * sizeof(float), cudaMemcpyDeviceToHost);

    // print result
    for (int i = 0; i < table.N; i++) {
        printf("%f ", outputH[i]);
    }
    printf("\n");

    // free device memory
    cudaFree(outputD);
    cudaFreeHost(outputH);

    return 0;
}