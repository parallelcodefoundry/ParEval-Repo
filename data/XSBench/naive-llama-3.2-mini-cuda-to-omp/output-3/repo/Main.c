// Main.cu (Translated from Main.cu)

#include "stdio.h"
#include "math.h"

// #pragma omp target map(tofrom:HM, HM[0:N]) teams distribute parallel
__global__
void main_kernel(float *device_to_host_time, float *kernel_time, float *host_to_device_time,
                  int num_iterations, int num_warmups) {
    // Note: This translation is an approximation and might not be exact.
    // The original CUDA code uses a kernel function with multiple threads, 
    // while the OpenMP-offload model requires each thread to execute the same operation.

    __shared__ float temp_host_to_device_time[1];

    int my_id = blockIdx.x * blockDim.x + threadIdx.x;

    if (my_id < num_iterations) {
        host_to_device_time[0] += 1.0f; // Accumulate device-to-host time
    }

    if (my_id < num_warmups) {
        kernel_time[0] += 1.0f; // Accumulate kernel time
    }
}

int main(int argc, char *argv[]) {
    // #pragma omp target map(from:HM, HM[0:N]) teams distribute parallel
    Inputs in;
    scanf("%d %ld %ld %d %s", &in.particles, &in.n_isotopes, &in.n_gridpoints,
           &in.num_iterations, in.HM);

    Profile profile;
    float device_to_host_time = 0.0f;
    float kernel_time = 0.0f;

    #pragma omp target map(to:HM[0:N]) teams distribute parallel
    {
        main_kernel(&device_to_host_time, &kernel_time, &profile.host_to_device_time,
                    in.num_iterations, in.num_warmups);
    }

    printf("host_to_device_ms,kernel_ms\n");
    printf("%f %f\n", profile.host_to_device_time, kernel_time);

    return 0;
}