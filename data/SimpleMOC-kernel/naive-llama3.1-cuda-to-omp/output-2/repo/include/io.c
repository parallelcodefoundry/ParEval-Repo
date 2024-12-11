#include <stdio.h>
#include <omp.h>

void __cudaCheckError(const char *file, const int line)
{
#ifdef CUDA_ERROR_CHECK
    // Removed CUDA-related code and replaced with OpenMP offloading macros
    #pragma offload target(mic:0) in(file) out(line)
    {
        printf("offload error: %s:%d\n", file, line);
        exit(1);
    }
#endif

    return;
}

Table buildExponentialTable(void)
{
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
#pragma offload target(mic:0) in(N, maxVal, dx) out(table.values[:2*N]) 
{
    #pragma omp parallel for num_threads(4) schedule(static, 1)
    for (int n = 0; n < N; n++)
    {
        float exponential = exp(-n * dx);
        table.values[2*n] = -exponential;
        table.values[2*n + 1] = 1 + (n * dx - 1) * exponential;
    }
}

    // assign data to table
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    return table;
}