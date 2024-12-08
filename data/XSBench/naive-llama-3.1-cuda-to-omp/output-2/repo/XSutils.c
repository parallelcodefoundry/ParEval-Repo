// File: XSutils.cu (translated to OpenMP-offload)

#define __OFFLOAD
#include <omp.h>
#include "XSbench_shared_header.h"

extern "C" void xs_utils_init(Inputs *in) {
    #pragma offload target(mic:0) in(in)
    #pragma omp target data map(in[in])
    {
        // Initialization code here...
    }
}

extern "C" void xs_utils_profile(Profile *profile, Inputs *in) {
    #pragma offload target(mic:0) in(profile,in)
    #pragma omp target data map(out[profile]) map(in[in])
    {
        // Profiling code here...
        profile->device_to_host_time = omp_get_wtime();
        print_profile(*profile, *in);
    }
}

extern "C" void xs_utils_lookup(int *result, double *data, int *index) {
    #pragma offload target(mic:0) in(result,data,index)
    #pragma omp target data map(out[result]) map(in[data]) map(in[index])
    {
        // Lookup code here...
    }
}