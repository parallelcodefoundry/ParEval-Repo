#include <omp.h>
#include <stdio.h>

#define CUDA_ERROR_CHECK 0

typedef struct {
    float dx;
    float maxVal;
    int N;
    float values[2 * 353];
} Table;

Table buildExponentialTable( void ) {
    // define table
    Table table;

    //float precision = 0.01;
    float maxVal = 10.0;    

    // compute number of arry values
    //int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * precision * 0.01 ) ) );
    int N = 353; 

    // store linear segment information (slope and y-intercept)
    for( int n = 0; n < N; n++ )
    {
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

void __cudaCheckError( const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    printf("cudaCheckError() not supported in OpenMP Offload\n");
#endif
 
    return;
}

// Initialize device data structures
Table buildExponentialTableDevice( void ) {
    Table table = buildExponentialTable();
    #pragma omp target map(to: table)
    {
        printf("Built exponential table on device\n");
    }
    return table;
}

int main() {
    #pragma omp parallel num_threads(1) default(none) shared(table)
    {
        #pragma omp single
        {
            Table table = buildExponentialTableDevice();
        }
    }

    // Initialize fine source and flux data structures
    Source *sources_h = initializeSources();
    Source_Arrays SA_d, SA_h;
    Sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h );

    // Set up CUDA kernel launch (replaced with OpenMP offload)
    #pragma omp target map(to: SA_d.fine_source_arr, SA_d.fine_flux_arr, SA_d.sigT_arr)

    __cudaCheckError(__FILE__, __LINE__);

    printf("Main function executed on host\n");

    return 0;
}