#ifndef XSBENCH_SHARED_HEADER_H
#define XSBENCH_SHARED_HEADER_H

// Header for shared utilities across XSBench versions

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Added for string manipulation functions
#include <math.h>
#include <assert.h>
#include <omp.h> // Include OpenMP header
#include <stdint.h>
#include <time.h> // For time measurement (if not using omp_get_wtime)

// Structures
typedef struct {
    int nthreads;
    long n_isotopes;
    long n_gridpoints;
    long lookups; // Changed to long to handle larger values
    char *HM;
    int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
    int hash_bins;
    long particles; // Changed to long to handle larger values
    int simulation_method;
    int binary_mode;
    int kernel_id;
    int num_iterations;
    int num_warmups;
    char *filename;
} Inputs;

typedef struct {
    double device_to_host_time;
    double kernel_time;
    double host_to_device_time;
} Profile;

// Function to print profiling information
inline void print_profile(Profile profile, Inputs in) {
    if (in.filename) {
        FILE *output = fopen(in.filename, "w");
        if (output == NULL) {
            fprintf(stderr, "Error opening file for writing: %s\n", in.filename);
            exit(1);
        }
        fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
        fprintf(output, "%f,%f,%f,%d,%d\n",
                profile.host_to_device_time * 1000,
                profile.kernel_time * 1000,
                profile.device_to_host_time * 1000,
                in.num_iterations,
                in.num_warmups);
        fclose(output);
    } else {
        printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
        printf("%f,%f,%f,%d,%d\n",
               profile.host_to_device_time * 1000,
               profile.kernel_time * 1000,
               profile.device_to_host_time * 1000,
               in.num_iterations,
               in.num_warmups);
    }
}


#endif // XSBENCH_SHARED_HEADER_H