Based on your CUDA code and the OpenMP offloading execution model, I will provide a translation of your code into OpenMP offloaded version. Here is the translated code:

```c
#include <omp.h>

#define N 353

// Structure to store table values
typedef struct {
    float dx;
    float maxVal;
    int N;
    float *values;
} Table;

Table buildExponentialTable() {
    Table table;
    
    // Define precision and compute number of array values
    float maxVal = 10.0;    
    int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * 0.01 ) ) );
    
    // Compute spacing
    float dx = maxVal / (float) N;
    
    // Allocate space for table values
    table.values = (float*) malloc(2*N*sizeof(float));
    
    // Store linear segment information (slope and y-intercept)
    #pragma omp parallel for
    for(int n=0; n<N; n++) {
        float exponential = exp(-n*dx);
        table.values[2*n] = -exponential;
        table.values[2*n+1] = 1 + (n*dx-1)*exponential;
    }
    
    // Assign data to table
    table.dx = dx;
    table.maxVal = maxVal-dx;
    table.N = N;
    
    return table;
}

void __ompCheckError(const char *file, const int line) {
#ifdef OMP_ERROR_CHECK
    #pragma omp taskwait
    if (omp_in_parallel()) {
        printf("Error in %s:%i: %s\n", file, line, omp_get_licenses());
        exit(-1);
    }
#endif
    
    return;
}

int main() {
    Table table = buildExponentialTable();
    
    // Print the built exponential table
    #pragma omp parallel for
    for(int i=0; i<2*N; i++) {
        printf("%f ", table.values[i]);
    }
    printf("\n");
    
    free(table.values);
    
    return 0;
}
```

Note that this code assumes a OpenMP compiler and runtime environment. The compilation command would be something like:

```bash
gcc -fopenmp -o output program.c
```

Also, the `CUDA_ERROR_CHECK` macro is replaced with `OMP_ERROR_CHECK` as OpenMP does not have a direct equivalent of CUDA error checking macros. The `#pragma omp parallel for` directives are used to offload loops to multiple threads or devices.