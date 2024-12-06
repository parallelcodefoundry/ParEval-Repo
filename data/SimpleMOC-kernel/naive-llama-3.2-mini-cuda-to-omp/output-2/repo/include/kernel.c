#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// Define the Table struct
typedef struct {
    float dx;
    float maxVal;
    int N;
    float* values;
} Table;

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

// Function to build a table of exponential values for linear interpolation
__attribute__((openmp targetDevice)) Table buildExponentialTableOpenMP() {
    #pragma omp target (device)
    Table table;

    // define table
    table.dx = 0.01;
    table.maxVal = 10.0;    

    // compute number of arry values
    int N = 353; 

    // compute spacing
    table.dx = 1 / N;

    // store linear segment information (slope and y-intercept)
    #pragma omp for parallel for schedule(static) private(n, exponential, value)
    for(int n = 0; n < N; n++) {
        exponential = exp( - n * table.dx );
        value = 1 + ( n * table.dx - 1 ) * exponential;
        table.values[ 2*n ] = - exponential;
        table.values[ 2*n + 1 ] = value;
    }

    // assign data to table
    table.N = N;

    return table;
}