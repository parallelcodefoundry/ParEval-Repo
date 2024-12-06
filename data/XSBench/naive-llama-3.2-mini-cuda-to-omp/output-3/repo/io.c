// io.cu (Translated from CUDA kernel)

#include "io.h"
#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 128 // Number of threads for each worker thread

void io_kernel(Inputs *in, Profile *profile) {
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int my_thread_id = omp_get_thread_num();
        if (my_thread_id >= in->nthreads) {
            return;
        }

        long offset = my_thread_id * in->particles;
        if (offset < 0 || offset >= in->n_particles) {
            return;
        }

        // Perform binary search for each particle
        while(in->lookups > 1){
            // Get the lower and upper bounds of the range to be searched
            long low = 1; // Start from first nuclide (index 0)
            long high = in->n_gridpoints; // Go up to last nuclide

            // Find the middle index of the range
            int mid = (low + high) / 2;
            if(in->binary_mode){
                // Search for particle with given binary input
                if((in->HM[my_thread_id*in->particles+offset] & (1 << mid)) > 0){
                    low = mid + 1; // Adjust lower bound to search on left side
                }
                else{
                    high = mid - 1; // Adjust upper bound to search on right side
                }
            }else{
                // Search for particle with given hash value
                if(in->HM[my_thread_id*in->particles+offset] == mid){
                    low = mid + 1; // No change needed in lower bound
                }
                else{
                    high = mid - 1; // No change needed in upper bound
                }
            }

            // Update the bounds for the next iteration
            if(low < high){
                long temp = low;
                low = high;
                high = temp;
            }
            in->lookups--;
        }

        // After the loop, 'in->HM[my_thread_id*in->particles+offset]' will hold the number of iterations
        profile->kernel_time += 1.0/in->nthreads; // Add time for this iteration to kernel time
    }

    #pragma omp critical
    {
        long num_iterations = in->num_iterations;
        if (num_iterations > 0) {
            profile->device_to_host_time += 1.0;
            profile->host_to_device_time += 0.0; // No need for this step
        }
    }

    #pragma omp barrier

    // Perform the host-to-device operation
    for (int i = 0; i < in->particles; ++i) {
        double *data = (double *)in->HM + my_thread_id*in->particles + i;
        data[my_thread_id] = profile->num_iterations;
    }

    // Update the number of iterations
    if(in->filename){
        FILE* output = fopen(in->filename, "a");
        fprintf(output, "%f,%f\n", profile->host_to_device_time*1000, profile->kernel_time*1000);
        fclose(output);
    }else{
        printf("%f,%f\n", profile->host_to_device_time*1000, profile->kernel_time*1000);
    }
}