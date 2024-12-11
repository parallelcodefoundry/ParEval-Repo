#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define CUDA_ERROR_CHECK 0

// Define a struct to represent the table data
typedef struct {
    float dx;
    float maxVal;
    int N;
    float values[2*N];
} Table;

Table buildExponentialTable(void) {
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
    #pragma omp parallel for num_threads(4) default(none) shared(N, dx, table.values)
    for (int n = 0; n < N; n++) {
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

void __ompCheckError(const char *file, const int line) {
#ifdef CUDA_ERROR_CHECK
    printf("Error at %s:%i\n", file, line);
#else
#endif
    return;
}

int main() {
    Table table = buildExponentialTable();
    printf("%f\t%f\t%d\n", table.dx, table.maxVal, table.N);

    // OpenMP offloading directives for memory management (assuming you want to transfer data from host to device)
    #pragma omp target data map(from:table) 
    {
        printf("On the device:\n");
        printf("%f\t%f\t%d\n", table.dx, table.maxVal, table.N);
    }

    return 0;
}