// Include necessary headers
#include "openmp.h"
#include <omp.h>
#include <iostream>

int main(int argc, char **argv) {
    // Initialize OpenMP environment
    #pragma omp offload target(nvidia)
    {   // Offload the following code to the NVIDIA GPU

        int test_run = 0;

        if (argc > 1 && argv[1][0] == '-') {
            // Parse command line options
            for (int i = 1; i < argc; ++i) {
                if (argv[i][0] == '-') {
                    if (strcmp(argv[i], "-test-run") == 0) {
                        test_run = 1;
                    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                        // Print help message
                        std::cout << "Usage: SimpleMOC-kernel [-test-run] [-h]\n";
                        return 0;
                    }
                }
            }
        }

        if (test_run) {
            // Run the test
            #pragma omp offload target(nvidia)
            {   // Offload the following code to the NVIDIA GPU

                Source_Arrays SA_h, SA_d;
                Source *sources = initialize_device_sources(SA_h, SA_d);
                Table table = buildExponentialTable();

                std::cout << "Running test...\n";

                // Call the kernels here (e.g., SimpleMOCKernel)
                #pragma omp offload target(nvidia)
                {   // Offload the following code to the NVIDIA GPU

                    // Kernel call
                    SimpleMOKernel(sources, SA_d);

                    // Sync memory
                    __cudaCheckError(__FILE__, __LINE__);
                }

            }  // Close offloaded region
        }
    }  // Close outer offloaded region

    return 0;
}