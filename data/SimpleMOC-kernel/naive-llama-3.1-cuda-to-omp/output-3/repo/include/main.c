#include <omp.h>
#include <cuda_runtime.h>

// ... (rest of the includes remain the same)

int main(int argc, char **argv) {
    // Initialize random number generator
    srand(0);

    InputParameters input;
    if (argc != 2) {
        printf("Usage: ./SimpleMOC-kernel <input_file>\n");
        exit(-1);
    }

    strcpy(input.filename, argv[1]);
    printf("Reading input file: %s\n", input.filename);

    // Initialize CUDA device
    int num_gpus = omp_get_max_threads();
    for (int i = 0; i < num_gpus; i++) {
        #pragma omp target map(to:i) device(i)
            cudaSetDevice(i);
        #pragma omp target data map(alloc:sources_h, SA_d)
            Source *sources_h;
            Source_Arrays *SA_d;
            sources_h = initialize_device_sources(input, SA_h, SA_d, sources_h);
        #pragma omp target map(from:output)
            // ... (rest of the main function remains the same)
    }

    return 0;
}

// ... (rest of the functions remain the same)

int initCUDA(void) {
    cudaError_t err = cudaSuccess;

    // Check if we can initialize CUDA
    err = cudaInit();
    if (err != cudaSuccess) {
        printf("Could not initialize CUDA: %s\n", cudaGetErrorString(err));
        return -1;
    }

    return 0;
}

void cleanupCUDA(void) {
    // ... (rest of the function remains the same)
}