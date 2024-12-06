#include <stdio.h>
#include <stdlib.h>

// Define the size of the array
#define N 353

// Function to compute exponential values for linear interpolation
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
    for(int n = 0; n < N; n++) {
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

// Function to initialize device sources with OpenMP offload
void __cudaCheckError(const char *file, const int line) {
#ifdef CUDA_ERROR_CHECK
    cudaError err = cudaGetLastError();
    if ( cudaSuccess != err )
    {
        fprintf(stderr, "cudaCheckError() failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString(err));
        exit(-1);
    }
 
    // More careful checking. However, this will affect performance.
    // Comment away if needed.
    err = cudaDeviceSynchronize();
    if (cudaSuccess != err)
    {
        fprintf(stderr, "cudaCheckError() with sync failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString(err));
        exit(-1);
    }
#endif
 
    return;
}

// Function to initialize device sources
Table buildExponentialTableOpenMP(int N) {
    #pragma omp target map(tofrom:table)
    Table table;
    
    for(int n = 0; n < N; n++) {
        float exponential = exp( - n * (10.0 / N) );
        table.values[ 2*n ] = - exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * (10.0 / N) - 1 ) * exponential;
    }
    
    table.dx = 10.0 / N;
    table.maxVal = 10.0 / N - table.dx;
    table.N = N;

    return table;
}

// Function to initialize device sources with OpenMP offload
Table __offload_buildExponentialTableOpenMP(int N) {
    Table table;
    
    #pragma omp target map(tofrom:table)
    for(int n = 0; n < N; n++) {
        float exponential = exp( - n * (10.0 / N) );
        table.values[ 2*n ] = - exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * (10.0 / N) - 1 ) * exponential;
    }
    
    table.dx = 10.0 / N;
    table.maxVal = 10.0 / N - table.dx;
    table.N = N;

    return table;
}

// Main function
void main() {
    int N = 353;
    Table table = buildExponentialTableOpenMP(N);
}