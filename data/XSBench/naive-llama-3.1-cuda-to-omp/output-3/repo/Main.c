#include "XSbench_shared_header.h"
#include <omp.h>
#include <stdio.h>

#define BLOCK_SIZE 256
#define MAX_GRID_SIZE (1 << 26)
#define THREADS_PER_BLOCK 256
#define SIMULATION_METHOD_EVENT 0
#define SIMULATION_METHOD_NAIVE 1

void print_usage() {
    printf("Usage: ./XSBench [options]\n");
    printf("Options:\n");
    printf("-m <method> : Use the event-based (default) or naive simulation method\n");
    printf("-s <size>   : Set input size (small, medium, large)\n");
}

int main(int argc, char** argv) {
    Inputs in;
    Profile profile;

    // Parse command line arguments
    int c;
    while ((c = getopt(argc, argv, "m:s:")) != -1) {
        switch (c) {
            case 'm':
                if (strcmp(optarg, "event") == 0) {
                    in.simulation_method = SIMULATION_METHOD_EVENT;
                } else if (strcmp(optarg, "naive") == 0) {
                    in.simulation_method = SIMULATION_METHOD_NAIVE;
                } else {
                    print_usage();
                    return -1;
                }
                break;
            case 's':
                if (strcmp(optarg, "small") == 0) {
                    in.lookups = 10;
                    in.num_iterations = 10000;
                    in.num_warmups = 100;
                } else if (strcmp(optarg, "medium") == 0) {
                    in.lookups = 25;
                    in.num_iterations = 20000;
                    in.num_warmups = 500;
                } else if (strcmp(optarg, "large") == 0) {
                    in.lookups = 50;
                    in.num_iterations = 40000;
                    in.num_warmups = 1000;
                } else {
                    print_usage();
                    return -1;
                }
                break;
            default:
                print_usage();
                return -1;
        }
    }

    // Initialize OpenMP variables
    omp_set_num_threads(in.nthreads);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            printf("Simulation method: %s\n", in.simulation_method == SIMULATION_METHOD_EVENT ? "event" : "naive");
            printf("Input size: %s\n", in.lookups == 10 ? "small" : in.lookups == 25 ? "medium" : "large");
        }
    }

    // Run simulation
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            profile.kernel_time = omp_get_wtime();
        }
        #pragma omp for schedule(static)
        for (int i = 0; i < in.num_iterations; i++) {
            // Simulation loop
        }
        if (tid == 0) {
            profile.kernel_time = omp_get_wtime() - profile.kernel_time;
        }
    }

    // Copy results to host and calculate performance metrics
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid == 0) {
            printf("Kernel time: %f seconds\n", profile.kernel_time);
            print_profile(profile, in);
        }
    }

    return 0;
}