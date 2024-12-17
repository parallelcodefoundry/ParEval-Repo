#include "SimpleMOC-kernel_header.h"

// Offload setup function for kernel launch
void init_kernel_offload(Input I, Source_Arrays * SA_h, Source_Arrays * SA_d) {
    // Allocate & Copy Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
#pragma omp offload target(s:cpu|gpu)
    {
        cudaMalloc((void **) &SA_d->fine_source_arr, N_fine * sizeof(float));
        cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);
    }

    // Allocate & Copy Fine Flux Data
    cudaMalloc((void **) &SA_d->fine_flux_arr, N_fine * sizeof(float));
#pragma omp offload target(s:cpu|gpu)
    {
        cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);
    }

    // Allocate & Copy SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
#pragma omp offload target(s:cpu|gpu)
    {
        cudaMalloc((void **) &SA_d->sigT_arr, N_sigT * sizeof(float));
        cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);
    }

    // Allocate Source Array Data
#pragma omp offload target(s:cpu|gpu)
    {
        Source * sources_d;
        cudaMalloc((void **) &sources_d, I.source_3D_regions * sizeof(Source));
        cudaMemcpy(sources_d, SA_h->source_arr, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);
    }
}

// Initializes global flux states to random numbers on device
__global__ void init_flux_states_offload(float * flux_states, int N_flux_states, Input I) {
    // Assign RNG state
    curandState * localState = &threadIdx.x;
    if( threadIdx.x == 0 )
        for( int i = 0; i < I.egroups; i++ )
            flux_states[blockIdx.x +i] = curand_uniform(localState);
}

// Offload function to build table of exponential values
void build_exponential_table_offload(Table * table) {
#pragma omp offload target(s:cpu|gpu)
    {
        float maxVal = 10.0;    
        int N = 353;   
        float dx = maxVal / (float) N;
        for( int n = 0; n < N; n++ )
        {
            // compute slope and y-intercept for ( 1 - exp(-x) )
            float exponential = exp( - n * dx );
            table->values[ 2*n ] = - exponential;
            table->values[ 2*n + 1 ] = 1 + ( n * dx - 1 ) * exponential;
        }
    }
}

// Offload function to initialize sources
void initialize_sources_offload(Input I, Source_Arrays * SA_h) {
#pragma omp offload target(s:cpu|gpu)
    {
        // Allocate Fine Source Data
        long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
        float * fine_source_arr = (float *) malloc( N_fine * sizeof(float));
        for( int i = 0; i < I.source_3D_regions; i++ )
            SA_h->source_arr[i].fine_source_id = i*I.fine_axial_intervals*I.egroups;

        // Allocate Fine Flux Data
        float * fine_flux_arr = (float *) malloc( N_fine * sizeof(float));
        for( int i = 0; i < I.source_3D_regions; i++ )
            SA_h->source_arr[i].fine_flux_id = i*I.fine_axial_intervals*I.egroups;

        // Allocate SigT Data
        long N_sigT = I.source_3D_regions * I.egroups;
        float * sigT_arr = (float *) malloc( N_sigT * sizeof(float));
        for( int i = 0; i < I.source_3D_regions; i++ )
            SA_h->source_arr[i].sigT_id = i * I.egroups;

        // Initialize fine source and flux to random numbers
        for( long i = 0; i < N_fine; i++ )
        {
            SA_h->fine_source_arr[i] = (float) rand() / RAND_MAX;
            SA_h->fine_flux_arr[i] = (float) rand() / RAND_MAX;
        }

        // Initialize SigT Values
        for( int i = 0; i < N_sigT; i++ )
            SA_h->sigT_arr[i] = (float) rand() / RAND_MAX;
    }
}

int main() {
    Input I;
    Source_Arrays * SA_h, * SA_d;

#pragma omp parallel target(s:cpu|gpu)
    {
        // Offload setup function for kernel launch
        init_kernel_offload(I, &SA_h, &SA_d);

        // Initialize global flux states to random numbers on device
        float * flux_states;
        int N_flux_states = 10000;
        cudaMalloc((void **) &flux_states, N_flux_states * I.egroups * sizeof(float));
#pragma omp offload target(s:cpu|gpu)
        {
            init_flux_states_offload(flux_states, N_flux_states, I);
        }

        // Build table of exponential values
        Table table;
        build_exponential_table_offload(&table);

        // Initialize sources
        initialize_sources_offload(I, &SA_h);
    }
}