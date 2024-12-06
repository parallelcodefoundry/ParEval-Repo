#include "omp.h"

__global__ void read_input(Inputs *in, Profile *profile) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < in->particles) {
        #pragma omp target map(tofrom(in[idx].HM)) map(tofrom(profile)) device(in[idx].HM)
        for (int i = 0; i < in->lookups; ++i) {
            // access shared memory
        }
    } else {
        #pragma omp target map(fromprofile) map(toin[idx]) device()
        // process input data
    }
}