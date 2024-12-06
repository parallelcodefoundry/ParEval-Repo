#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include "Table.h"

// Build a table of exponential values for linear interpolation
Table buildExponentialTable( void )
{
    // define table
    Table table;

    //float precision = 0.01;
    float maxVal = 10.0;   // modified to be a float

    // compute number of arry values
    //int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * precision * 0.01 ) ) );
    int N = 353; 

    // compute spacing
    float dx;
#ifdef __CUDACC__
    cudaError_t err = cudaMalloc((void **)&dx, sizeof(float));
    if( err != cudaSuccess )
        exit(-1);
#else
    dx = maxVal / (float) N;
#endif

    // store linear segment information (slope and y-intercept)
    #ifdef __CUDACC__
    for(int n=0; n<N; n++)
    {
        float exponential = expf( -n * dx );
        table.values[ 2*n ] = -exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
    }
    #else
    for(int n=0; n<N; n++)
    {
        float exponential = exp( -n * dx );
        table.values[ 2*n ] = -exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
    }
    #endif

    // assign data to table
    #ifdef __CUDACC__
    cudaMalloc((void **)&table.dx, sizeof(float));
    cudaMemcpy(table.dx, &dx, sizeof(float), cudaMemcpyHostToDevice);
    cudaMalloc((void **)&table.maxVal, sizeof(float));
    cudaMemcpy(table.maxVal, &maxVal, sizeof(float), cudaMemcpyHostToDevice);
    table.N = N;
    #else
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;
    #endif

    return table;
}