#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

// Function to print program logo
void logo(int version) {
    std::cout << "   __           __        ___        __   __           ___  __        ___     \n"
              << "  /__` |  |\\/| |__) |    |__   |\\/| /  \\ /  ` __ |__/ |__  |__) |\\ | |__  |   \n"
              << "  .__/ |  |  | |    |___ |___  |  | \\__/ \\__,    |  \\ |___ |  \\ | \\| |___ |___\n"
              << "\n"
              << "                         *****************************   *****************************  *****************************\n"
              << "                        ******************************   ******************************  ******************************\n"
              << "                        *************************     *************************  *************************\n"
              << "                        *************************     *************************  *************************\n"
              << "                        ******************************   ******************************  ******************************\n"
              << "                         *****************************   *****************************  *****************************\n"
              << "\n";
    std::cout << std::setw(40) << "Developed at" << std::endl;
    std::cout << std::setw(40) << "The Massachusetts Institute of Technology" << std::endl;
    std::cout << std::setw(40) << "and" << std::endl;
    std::cout << std::setw(40) << "Argonne National Laboratory" << std::endl;
    std::cout << "\n";
    std::cout << "Version: " << version << std::endl;
    std::cout << "\n";
}

// Function to print section titles in center of 80 char terminal
void center_print(const std::string& s, int width) {
    int length = s.length();
    int i;
    for (i = 0; i <= (width - length) / 2; i++) {
        std::cout << " ";
    }
    std::cout << s << std::endl;
}

// Function to print a border
void border_print(void) {
    std::cout << "===============================================================================" << std::endl;
}

// Function to print comma separated integers - for ease of reading
void fancy_int(int a) {
    if (a < 1000) {
        std::cout << a << std::endl;
    } else if (a >= 1000 && a < 1000000) {
        std::cout << a / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    } else if (a >= 1000000 && a < 1000000000) {
        std::cout << a / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    } else if (a >= 1000000000) {
        std::cout << a / 1000000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000000) / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    } else {
        std::cout << a << std::endl;
    }
}

// Function to print out the summary of User input
void print_input_summary(Input I) {
    center_print("INPUT SUMMARY", 79);
    border_print();

    std::cout << std::setw(25) << "CUDA Device: " << Kokkos::DefaultExecutionSpace::concurrency() << std::endl;
    std::cout << std::setw(25) << "Energy Groups: " << I.egroups << std::endl;
    std::cout << std::setw(25) << "2D Source Regions: " << I.source_2D_regions << std::endl;
    std::cout << std::setw(25) << "Coarse Axial Intervals: " << I.coarse_axial_intervals << std::endl;
    std::cout << std::setw(25) << "Fine Axial Intervals: " << I.fine_axial_intervals << std::endl;
    std::cout << std::setw(25) << "Axial Decomposition: " << I.decomp_assemblies_ax << std::endl;
    std::cout << std::setw(25) << "3D Source Regions: " << I.source_3D_regions << std::endl;
    std::cout << std::setw(25) << "Segments: ";
    fancy_int(I.segments);
    std::cout << std::setw(25) << "Random Number Streams: ";
    fancy_int(I.streams);
    std::cout << std::setw(25) << "Memory Estimate (MB): " << mem_estimate(I) << std::endl;
    std::cout << std::setw(25) << "Segments per CUDA block: " << I.seg_per_thread << std::endl;
#ifdef TABLE
    std::cout << std::setw(25) << "Exponential Table: " << "ON" << std::endl;
#else
    std::cout << std::setw(25) << "Exponential Table: " << "OFF" << std::endl;
#endif
    border_print();
}

// Function to read command line inputs and apply options
void read_CLI(int argc, char* argv[], Input* input) {
    // Collect Raw Input
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // nthreads (-t)
        if (arg == "-t") {
            if (++i < argc) {
                input->nthreads = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // segments (-s)
        else if (arg == "-s") {
            if (++i < argc) {
                input->segments = std::stol(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // egroups (-e)
        else if (arg == "-e") {
            if (++i < argc) {
                input->egroups = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // segments per thread (-p)
        else if (arg == "-p") {
            if (++i < argc) {
                input->seg_per_thread = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // CUDA Device Number (-d)
        else if (arg == "-d") {
            if (++i < argc) {
                int device_id = std::stoi(argv[i]);
                Kokkos::initialize(Kokkos::InitArguments(device_id));
            } else {
                print_CLI_error();
            }
        } else {
            print_CLI_error();
        }
    }
}

// Function to print error to screen, inform program options
void print_CLI_error(void) {
    std::cout << "Usage: ./SimpleMOC <options>\n";
    std::cout << "Options include:\n";
    std::cout << "  -t <threads>          Number of OpenMP threads to run\n";
    std::cout << "  -s <segments>         Number of segments to process\n";
    std::cout << "  -e <energy groups>    Number of energy groups\n";
    std::cout << "  -p <segs per thread>  Number of segments per CUDA Block\n";
    std::cout << "  -d <CUDA device ID>   CUDA GPU device ID number\n";
    std::cout << "See readme for full description of default run values\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    int version = 4;

    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    // Calculate Number of 3D Source Regions
    I.source_3D_regions = (int)ceil((double)I.source_2D_regions * I.coarse_axial_intervals / I.decomp_assemblies_ax);

    logo(version);

    print_input_summary(I);

    center_print("INITIALIZATION", 79);
    border_print();

    // Build Source Data
    std::cout << "Building Source Data Arrays...\n";
    Source_Arrays SA_h, SA_d;
    Source* sources_h = initialize_sources(I, &SA_h);
    Source* sources_d;
    Kokkos::View<Source*> sources_d_view("sources_d", I.source_3D_regions);
    Kokkos::deep_copy(sources_d_view, sources_h);

    // Build Exponential Table
    Table* table_d = nullptr;
#ifdef TABLE
    std::cout << "Building Exponential Table...\n";
    Table table = buildExponentialTable();
    Kokkos::View<Table*> table_d_view("table_d", 1);
    Kokkos::deep_copy(table_d_view, &table);
#endif

    // Setup CUDA blocks / threads
    int n_blocks = sqrt(I.segments);
    Kokkos::View<int*> blocks_view("blocks", 2);
    blocks_view(0) = n_blocks;
    blocks_view(1) = n_blocks;
    if (blocks_view(0) * blocks_view(1) < I.segments) {
        blocks_view(0)++;
    }
    if (blocks_view(0) * blocks_view(1) < I.segments) {
        blocks_view(1)++;
    }
    assert(blocks_view(0) * blocks_view(1) >= I.segments);

    // Setup CUDA RNG on Device
    std::cout << "Setting up CUDA RNG...\n";
    Kokkos::View<curandState*> RNG_states("RNG_states", I.streams);
    Kokkos::parallel_for(Kokkos::RangePolicy<>(0, I.streams), KOKKOS_LAMBDA(const int i) {
        curand_init(1234, i, 0, &RNG_states(i));
    });

    // Allocate Some Flux State vectors to randomly pick from
    std::cout << "Setting up Flux State Vectors...\n";
    int N_flux_states = 10000;
    assert(I.segments >= N_flux_states);
    Kokkos::View<float*> flux_states("flux_states", N_flux_states * I.egroups);
    Kokkos::parallel_for(Kokkos::RangePolicy<>(0, N_flux_states), KOKKOS_LAMBDA(const int i) {
        for (int j = 0; j < I.egroups; j++) {
            flux_states(i * I.egroups + j) = curand_uniform(&RNG_states(i % I.streams));
        }
    });

    std::cout << "Initialization Complete.\n";
    border_print();
    center_print("SIMULATION", 79);
    border_print();
    std::cout << "Attentuating fluxes across segments...\n";

    // Run Simulation Kernel Loop
    Kokkos::Timer timer;
    Kokkos::parallel_for(Kokkos::RangePolicy<>(0, I.segments / I.seg_per_thread), KOKKOS_LAMBDA(const int i) {
        int blockId = i * I.seg_per_thread;
        for (int j = 0; j < I.seg_per_thread; j++) {
            // Assign RNG state
            curandState* localState = &RNG_states(blockId % I.streams);

            // Thread Local (i.e., specific to E group) variables
            float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

            // Randomized variables (common accross all thread within block)
            int state_flux_id = curand(localState) % N_flux_states;
            int QSR_id = curand(localState) % I.source_3D_regions;
            int FAI_id = curand(localState) % I.fine_axial_intervals;

            float* state_flux = &flux_states(state_flux_id * I.egroups);

            // Attenuate Segment
            float dz = 0.1f;
            float zin = 0.3f;
            float weight = 0.5f;
            float mu = 0.9f;
            float mu2 = 0.3f;
            float ds = 0.7f;

            const int egroups = I.egroups;

            // load fine source region flux vector
            float* FSR_flux = &SA_h.fine_flux_arr[SA_h.fine_flux_id + FAI_id * egroups];

            if (FAI_id == 0) {
                float* f2 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id) * egroups];
                float* f3 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id + 1) * egroups];
                // cycle over energy groups
                // load neighboring sources
                float y2 = f2[0];
                float y3 = f3[0];

                // do linear "fitting"
                float c0 = y2;
                float c1 = (y3 - y2) / dz;

                // calculate q0, q1, q2
                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            } else if (FAI_id == I.fine_axial_intervals - 1) {
                float* f1 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id - 1) * egroups];
                float* f2 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id) * egroups];
                // cycle over energy groups
                // load neighboring sources
                float y1 = f1[0];
                float y2 = f2[0];

                // do linear "fitting"
                float c0 = y2;
                float c1 = (y2 - y1) / dz;

                // calculate q0, q1, q2
                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            } else {
                float* f1 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id - 1) * egroups];
                float* f2 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id) * egroups];
                float* f3 = &SA_h.fine_source_arr[SA_h.fine_source_id + (FAI_id + 1) * egroups];
                // cycle over energy groups
                // load neighboring sources
                float y1 = f1[0];
                float y2 = f2[0];
                float y3 = f3[0];

                // do quadratic "fitting"
                float c0 = y2;
                float c1 = (y1 - y3) / (2.0f * dz);
                float c2 = (y1 - 2.0f * y2 + y3) / (2.0f * dz * dz);

                // calculate q0, q1, q2
                q0 = c0 + c1 * zin + c2 * zin * zin;
                q1 = c1 + 2.0f * c2 * zin;
                q2 = c2;
            }

            // load total cross section
            sigT = SA_h.sigT_arr[0];

            // calculate common values for efficiency
            tau = sigT * ds;
            sigT2 = sigT * sigT;

#ifdef TABLE
            interpolateTable(table_d, tau, &expVal);
#else
            expVal = 1.0f - expf(-tau);  // EXP function is faster than table lookup
#endif

            // Flux Integral

            // Re-used Term
            reuse = tau * (tau - 2.0f) + 2.0f * expVal / (sigT * sigT2);

            // add contribution to new source flux
            flux_integral = (q0 * tau + (sigT * state_flux[0] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.0f) + 6.0f) - 6.0f * expVal) / (3.0f * sigT2 * sigT2);

            // Prepare tally
            tally = weight * flux_integral;

            // SHOULD BE ATOMIC HERE!
            // FSR_flux[0] += tally;
            FSR_flux[0] += (float)tally;

            // Term 1
            t1 = q0 * expVal / sigT;
            // Term 2
            t2 = q1 * mu * (tau - expVal) / sigT2;
            // Term 3
            t3 = q2 * mu2 * reuse;
            // Term 4
            t4 = state_flux[0] * (1.0f - expVal);
            // Total psi
            state_flux[0] = t1 + t2 + t3 + t4;
        }
    });

    std::cout << "Simulation Complete.\n";

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double tpi = ((double)(timer.seconds() / I.segments / (double)I.egroups)) * 1.0e9;
    std::cout << std::setw(25) << "Runtime: " << timer.seconds() << " seconds" << std::endl;
    std::cout << std::setw(25) << "Time per Intersection: " << tpi << " ns" << std::endl;
    border_print();

    return 0;
}