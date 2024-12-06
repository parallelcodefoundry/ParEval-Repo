#include "io.h"
#include <omp.h>

void openmp_offload_io(double* buffer, int size) {
    #pragma omp parallel for num_threads(get_num_threads())
    for (int i = 0; i < size; ++i) {
        __device__ void read_device() {
            // device code here
        }

        #ifdef OFFLOAD
        __host__ __device__ void write_device(double value) {
            buffer[i] = value;
        }
        #else
        __host__ void write_host(double value) {
            buffer[i] = value;
        }
        #endif

        read_device();
        #if defined(OFFLOAD)
        if (get_num_threads() == 1) {
            write_host(buffer[i]);
        } else {
            #pragma omp critical
            write_host(buffer[i]);
        }
        #else
        if (get_num_threads() > 1) {
            #pragma omp critical
            write_host(buffer[i]);
        } else {
            write_host(buffer[i]);
        }
        #endif

    }
}

void openmp_offload_io_buffered(double* buffer, int size) {
    #pragma omp parallel for num_threads(get_num_threads())
    for (int i = 0; i < size; ++i) {
        __device__ void read_device() {
            // device code here
        }

        double value;
        #ifdef OFFLOAD
        __host__ __device__ void read_device(double* value) {
            *value = buffer[i];
        }
        #else
        __host__ void read_host(double* value) {
            *value = buffer[i];
        }
        #endif

        #if defined(OFFLOAD)
        if (get_num_threads() == 1) {
            value = buffer[i];
        } else {
            #pragma omp critical
            value = buffer[i];
        }
        #else
        if (get_num_threads() > 1) {
            #pragma omp critical
            value = buffer[i];
        } else {
            value = buffer[i];
        }
        #endif

        read_device(value);
    }
}