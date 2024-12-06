#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to calculate exponential values for linear interpolation
Table buildExponentialTable( void )
{
    Table table;

    float maxVal = 10.0;  
    int N = 353; 

    float dx = maxVal / (float) N;

    #pragma omp parallel for
    for( int n = 0; n < N; n++ )
    {
        float exponential = exp( - n * dx );
        table.values[ 2*n ] = - exponential;
        table.values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
    }

    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    return table;
}

// Function to initialize device sources
Source * initialize_device_sources( Input I, Source_Arrays * SA_h, Source_Arrays * SA_d, Source * sources_h )
{
    #pragma omp parallel for
    {
        long n = 0; // assuming N is a global variable

        cudaMalloc((void **) &SA_d->fine_source_arr, N * sizeof(float));
        cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr + n*N, N * sizeof(float), cudaMemcpyHostToDevice);

        cudaMalloc((void **) &SA_d->fine_flux_arr, N * sizeof(float));
        cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr + n*N, N * sizeof(float), cudaMemcpyHostToDevice);

        cudaMalloc((void **) &SA_d->sigT_arr, N * sizeof(float));
        cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr + n*N, N * sizeof(float), cudaMemcpyHostToDevice);

        cudaMalloc((void **) &sources_d, I.source_3D_regions * sizeof(Source));
        cudaMemcpy(sources_d, sources_h + n*I.source_3D_regions, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);
    }

    return sources_d;
}

// Function to initialize device sources in a multi-threaded loop
void initialize_device_sources( Input I, Source_Arrays * SA_h, Source_Arrays * SA_d, Source * sources_h )
{
    #pragma omp parallel for schedule(dynamic) num_threads(omp_get_max_threads())
    {
        long n = 0; // assuming N is a global variable

        cudaMalloc((void **) &SA_d->fine_source_arr, I.source_3D_regions * sizeof(float));
        cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr + n*I.source_3D_regions, I.source_3D_regions * sizeof(float), cudaMemcpyHostToDevice);

        cudaMalloc((void **) &SA_d->fine_flux_arr, I.source_3D_regions * sizeof(float));
        cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr + n*I.source_3D_regions, I.source_3D_regions * sizeof(float), cudaMemcpyHostToDevice);

        cudaMalloc((void **) &SA_d->sigT_arr, I.source_3D_regions * sizeof(float));
        cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr + n*I.source_3D_regions, I.source_3D_regions * sizeof(float), cudaMemcpyHostToDevice);

        cudaMalloc((void **) &sources_d, I.source_3D_regions * sizeof(Source));
        cudaMemcpy(sources_d, sources_h + n*I.source_3D_regions, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);
    }
}