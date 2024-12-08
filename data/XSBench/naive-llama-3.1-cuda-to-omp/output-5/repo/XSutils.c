// XSutils_translated.cpp

#include <omp.h>
#include "XSbench_shared_header.h"

#define CUDA_KERNEL LaunchKernel

void _launch_kernel(char * kernel_name, int num_iterations) {
    omp_set_num_threads(1);
    #pragma omp target data map(kernel: char* kernel_name, iterations: int num_iterations)
    {
        kernel = kernel_name;
        iterations = num_iterations;
    }
    #pragma offload transfer kernel
    for (int i=0; i<iterations; i++) {
        // Call the kernel using offloaded function pointers
        CUDA_KERNEL(kernel);
    }
}

void _call_kernel(char * kernel_name, int num_iterations) {
    omp_set_num_threads(1);
    #pragma omp target data map(kernel: char* kernel_name, iterations: int num_iterations)
    {
        kernel = kernel_name;
        iterations = num_iterations;
    }
    #pragma offload transfer kernel
    for (int i=0; i<iterations; i++) {
        // Call the kernel using offloaded function pointers
        CUDA_KERNEL(kernel);
    }
}

void _kernel_launch(char * kernel_name, int num_iterations) {
    omp_set_num_threads(1);
    #pragma omp target data map(kernel: char* kernel_name, iterations: int num_iterations)
    {
        kernel = kernel_name;
        iterations = num_iterations;
    }
    #pragma offload transfer kernel
    for (int i=0; i<iterations; i++) {
        // Call the kernel using offloaded function pointers
        CUDA_KERNEL(kernel);
    }
}

void _offload_kernel(char * kernel_name, int num_iterations) {
    omp_set_num_threads(1);
    #pragma omp target data map(kernel: char* kernel_name, iterations: int num_iterations)
    {
        kernel = kernel_name;
        iterations = num_iterations;
    }
    #pragma offload transfer kernel
    for (int i=0; i<iterations; i++) {
        // Call the kernel using offloaded function pointers
        CUDA_KERNEL(kernel);
    }
}