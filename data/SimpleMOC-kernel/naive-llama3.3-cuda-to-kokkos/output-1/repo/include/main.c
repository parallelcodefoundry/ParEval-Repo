#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <cmath>

int main(int argc, char* argv[]) {
    int version = 4;

    Kokkos::initialize(argc, argv);
    {
        srand(time(NULL));

        Input I = set_default_input();
        read_CLI(argc, argv, &I);

        // Calculate Number of 3D Source Regions
        I.source_3D_regions = (int)ceil((double)I.source_2D_regions *
                                        I.coarse_axial_intervals / I.decomp_assemblies_ax);

        logo(version);

        print_input_summary(I);

        std::cout << "INITIALIZATION" << std::endl;
        std::cout << "===============" << std::endl;

        // Build Source Data
        std::cout << "Building Source Data Arrays..." << std::endl;
        Source_Arrays SA_h, SA_d;
        Source* sources_h = initialize_sources(I, &SA_h);
        Source* sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);

        // Build Exponential Table
        Table* table_d = nullptr;
#ifdef TABLE
        std::cout << "Building Exponential Table..." << std::endl;
        Table table = buildExponentialTable();
        table_d = new Table;
        *table_d = table;
#endif

        // Setup CUDA RNG on Device
        std::cout << "Setting up CUDA RNG..." << std::endl;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);

        // Allocate Some Flux State vectors to randomly pick from
        std::cout << "Setting up Flux State Vectors..." << std::endl;
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);
        float* flux_states = new float[N_flux_states * I.egroups];
        for (int i = 0; i < N_flux_states * I.egroups; i++) {
            flux_states[i] = dis(gen);
        }

        std::cout << "Initialization Complete." << std::endl;
        std::cout << "===============" << std::endl;
        std::cout << "SIMULATION" << std::endl;
        std::cout << "===========" << std::endl;

        // Run Simulation Kernel Loop
        Kokkos::Timer timer;
        run_kernel(I, sources_d, SA_d, table_d, flux_states, N_flux_states);
        double time = timer.seconds();

        std::cout << "Simulation Complete." << std::endl;
        std::cout << "===============" << std::endl;
        std::cout << "RESULTS SUMMARY" << std::endl;
        std::cout << "===============" << std::endl;

        double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
        std::cout << "Runtime: " << time << " seconds" << std::endl;
        std::cout << "Time per Intersection: " << tpi << " ns" << std::endl;
        std::cout << "===============" << std::endl;

        delete[] flux_states;
        delete sources_h;
        delete sources_d;
        delete[] SA_h.fine_flux_arr;
        delete[] SA_h.fine_source_arr;
        delete[] SA_h.sigT_arr;
        delete[] SA_d.fine_flux_arr;
        delete[] SA_d.fine_source_arr;
        delete[] SA_d.sigT_arr;
        delete table_d;
    }
    Kokkos::finalize();
    return 0;
}