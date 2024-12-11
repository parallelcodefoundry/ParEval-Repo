// include/main.cu (translated to OpenMP Offload)

#include <omp.h>
#include <cuda_runtime_api.h>

#define cudaCheckError( msg ) \
    do { \
        cudaError err = cudaMemcpy( dst, src, size, cudaMemcpyHostToDevice ); \
        if( cudaSuccess != err ) { \
            fprintf( stderr, "CUDA error: %s\n", msg ); \
            exit( -1 ); \
        } \
    } while ( 0 )

void __cudaCheckError( const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    cudaError err = cudaGetLastError();
    if ( cudaSuccess != err )
    {
        fprintf( stderr, "cudaCheckError() failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString( err ) );
        exit( -1 );
    }
 
    // More careful checking. However, this will affect performance.
    // Comment away if needed.
    err = cudaDeviceSynchronize();
    if( cudaSuccess != err )
    {
        fprintf( stderr, "cudaCheckError() with sync failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString( err ) );
        exit( -1 );
    }
#endif
 
    return;
}

int main()
{
    // Initialize OpenMP Offload
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    omp_set_default_device(deviceCount-1);  // Set default device to the last one

    // Check if CUDA is available on this host
    int major, minor;
    cudaDriverGetVersion(&major, &minor);
    if (major != 2010 || minor < 3) {
        fprintf(stderr, "This code requires CUDA 10.3 or higher\n");
        return 1;
    }

    // Initialize variables for exponential table
    Table table;

    // Build the exponential table on the host
    table = buildExponentialTable();

    // Create memory space on the device for the table
    float *table_d;
    cudaMalloc(&table_d, sizeof(Table));

    // Copy the table from host to device
    cudaMemcpy(table_d, &table, sizeof(Table), cudaMemcpyHostToDevice);

    // Offload kernel execution to the device
    #pragma omp target teams distribute parallel for num_teams(4) default(none) map(from:table)
    {
        float maxVal = 10.0f;
        int N = table.N;
        float dx = table.dx;

        // Compute number of array values
        int arrLen = (int)(maxVal * sqrt(1.0 / (8.0 * 0.01 * dx)));

        // Store linear segment information (slope and y-intercept)
        for(int n=0; n<arrLen; n++) {
            float exponential = exp(-n*dx);
            table.values[2*n] = -exponential;
            table.values[2*n+1] = 1 + (n*dx-1) * exponential;
        }
    }

    // Synchronize device and print result
    cudaDeviceSynchronize();
    printf("Exponential Table:\n");
    for(int i=0; i<table.N*2; i++)
        printf("%f ", table.values[i]);

    return 0;
}