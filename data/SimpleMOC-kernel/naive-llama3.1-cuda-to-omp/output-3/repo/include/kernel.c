#include <omp.h>
#include <cuda_runtime.h>

#define CUDA_ERROR_CHECK 1 // uncomment this line if you want error checking

void __cudaCheckError( const char *file, const int line ) {
#ifdef CUDA_ERROR_CHECK
    cudaError err = cudaGetLastError();
    if ( cudaSuccess != err )
    {
        fprintf(stderr, "cudaCheckError() failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString(err) );
        exit(-1);
    }
#endif
 
    return;
}

// Builds a table of exponential values for linear interpolation
Table buildExponentialTable( void )
{
    // define table
    Table table;

    #pragma omp target offload map(arg:table)
    {
        float precision = 0.01;
        float maxVal = 10.0;    

        // compute number of arry values
        int N = (int) (maxVal * sqrt(1.0 / (8.0 * precision * 0.01)));

        // compute spacing
        float dx = maxVal / (float) N;

        // store linear segment information (slope and y-intercept)
        for(int n = 0; n < N; n++)
        {
            // compute slope and y-intercept for ( 1 - exp(-x) )
            float exponential = exp( - n * dx );
            table.values[2*n] = -exponential;
            table.values[2*n + 1] = 1 + (n * dx - 1) * exponential;
        }

        // assign data to table
        table.dx = dx;
        table.maxVal = maxVal - table.dx;
        table.N = N;
    }

    return table;
}

// Initializes device sources and arrays for the kernel execution
Source* initialize_device_sources( Input I, Source_Arrays *SA_h, Source_Arrays *SA_d, Source *sources_h )
{
    // Allocate & Copy Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    cudaMalloc((void **) &SA_d->fine_source_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy Fine Flux Data
    cudaMalloc((void **) &SA_d->fine_flux_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    cudaMalloc((void **) &SA_d->sigT_arr, N_sigT * sizeof(float));
    cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy Source Array Data
    Source* sources_d;
    cudaMalloc((void **) &sources_d, I.source_3D_regions * sizeof(Source));
    cudaMemcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);

    return sources_d;
}

// Initializes device sources and arrays for the kernel execution (with OpenMP Offload)
#pragma omp target offload map(arg:sources_d) in(SA_h) out(SA_d)
void initialize_device_sources_offload( Input I, Source_Arrays *SA_h, Source_Arrays *SA_d )
{
    // Allocate & Copy Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    cudaMalloc((void **) &SA_d->fine_source_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy Fine Flux Data
    cudaMalloc((void **) &SA_d->fine_flux_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    cudaMalloc((void **) &SA_d->sigT_arr, N_sigT * sizeof(float));
    cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy Source Array Data
    Source* sources_d = (Source*)malloc(I.source_3D_regions * sizeof(Source));
    cudaMalloc((void **) &sources_d, I.source_3D_regions * sizeof(Source));
    cudaMemcpy(sources_d, SA_h->source_array, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);

    // Return the device source array
    return sources_d;
}

// Kernel function using OpenMP Offload for execution
#pragma omp declare target
void kernel( Input I, Source_Arrays* SA )
{
    #pragma omp target offload map(arg:SA)
    {
        buildExponentialTable();
        initialize_device_sources_offload(I, &SA_h, &SA_d);
        // Perform other computations here...
    }
}

// Call the kernel function
void execute_kernel( Input I, Source_Arrays* SA )
{
    #pragma omp target offload map(arg:SA)
    {
        kernel(I, SA);
    }
}