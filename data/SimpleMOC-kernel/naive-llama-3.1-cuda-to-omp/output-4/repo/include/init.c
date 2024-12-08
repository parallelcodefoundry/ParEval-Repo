#include <omp.h>
#include <cuda_runtime.h>

#define CUDA_ERROR_CHECK

Table buildExponentialTable( void )
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

    #pragma omp target teams distribute parallel for
    for(int n = 0; n < N; n++)
    {
        // store linear segment information (slope and y-intercept)
        float exponential = exp( - n * dx );
        table.values[ 2*n ] = - exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
    }

    #pragma omp target update from(table)

    // assign data to table
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    return table;
}

void __omp_cudaCheckError( const char *file, const int line )
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

Source * initialize_device_sources( Input I, Source_Arrays * SA_h, Source_Arrays * SA_d, Source * sources_h )
{
    #pragma omp target teams distribute parallel for
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    cudaMalloc((void **) &SA_d->fine_source_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    #pragma omp target teams distribute parallel for
    cudaMalloc((void **) &SA_d->fine_flux_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    #pragma omp target teams distribute parallel for
    long N_sigT = I.source_3D_regions * I.egroups;
    cudaMalloc((void **) &SA_d->sigT_arr, N_sigT * sizeof(float));
    cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);

    #pragma omp target teams distribute parallel for
    Source * sources_d;
    cudaMalloc((void **) &sources_d, I.source_3D_regions * sizeof(Source));
    cudaMemcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);

    return sources_d;
}