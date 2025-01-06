#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

int main(int argc, char* argv[]) {
    Kokkos::initialize(argc, argv);
    {
        int version = 4;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        Input I = set_default_input();
        read_CLI(argc, argv, &I);

        logo(version);

        print_input_summary(I);

        std::cout << "INITIALIZATION" << std::endl;

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
        table_d = new Table();
        *table_d = table;
#endif

        // Setup Kokkos execution space
        Kokkos::DefaultExecutionSpace exec_space;
        Kokkos::DefaultExecutionSpace::print_configuration(std::cout);

        // Allocate Some Flux State vectors to randomly pick from
        std::cout << "Setting up Flux State Vectors..." << std::endl;
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);
        Kokkos::View<float*> flux_states("flux_states", N_flux_states * I.egroups);
        Kokkos::parallel_for(Kokkos::RangePolicy<>(exec_space, 0, N_flux_states * I.egroups), KOKKOS_LAMBDA(const int i) {
            flux_states(i) = dis(gen);
        });

        std::cout << "Initialization Complete." << std::endl;

        std::cout << "SIMULATION" << std::endl;

        // Run Simulation Kernel Loop
        Kokkos::Timer timer;
        Kokkos::parallel_for(Kokkos::RangePolicy<>(exec_space, 0, I.segments / I.seg_per_thread), KOKKOS_LAMBDA(const int i) {
            int blockId = i * I.seg_per_thread;
            for (int j = 0; j < I.seg_per_thread; j++) {
                int idx = blockId + j;
                if (idx >= I.segments) break;

                // Assign RNG state
                std::mt19937 localState = gen;

                // Thread Local (i.e., specific to E group) variables
                float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

                // Randomized variables (common accross all thread within block)
                int state_flux_id = localState() % N_flux_states;
                int QSR_id = localState() % I.source_3D_regions;
                int FAI_id = localState() % I.fine_axial_intervals;

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
                float* FSR_flux = &SA_d.fine_flux_arr[sources_h[QSR_id].fine_flux_id + FAI_id * egroups];

                if (FAI_id == 0) {
                    float* f2 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id) * egroups];
                    float* f3 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id + 1) * egroups];
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
                }
                else if (FAI_id == I.fine_axial_intervals - 1) {
                    float* f1 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id - 1) * egroups];
                    float* f2 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id) * egroups];
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
                }
                else {
                    float* f1 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id - 1) * egroups];
                    float* f2 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id) * egroups];
                    float* f3 = &SA_d.fine_source_arr[sources_h[QSR_id].fine_source_id + (FAI_id + 1) * egroups];
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
                sigT = SA_d.sigT_arr[sources_h[QSR_id].sigT_id];

                // calculate common values for efficiency
                tau = sigT * ds;
                sigT2 = sigT * sigT;

#ifdef TABLE
                interpolateTable(table_d, tau, &expVal);
#else
                expVal = 1.0f - expf(-tau);
#endif

                // Flux Integral

                // Re-used Term
                reuse = tau * (tau - 2.0f) + 2.0f * expVal / (sigT * sigT2);

                // add contribution to new source flux
                flux_integral = (q0 * tau + (sigT * state_flux[0] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.0f) + 6.0f) - 6.0f * expVal) / (3.0f * sigT2 * sigT2);

                // Prepare tally
                tally = weight * flux_integral;

                // SHOULD BE ATOMIC HERE!
                //FSR_flux[0] += tally;
                FSR_flux[0] += tally;

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

        double time = timer.seconds();

        std::cout << "Simulation Complete." << std::endl;

        std::cout << "RESULTS SUMMARY" << std::endl;

        double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
        std::cout << "Runtime: " << time << " seconds" << std::endl;
        std::cout << "Time per Intersection: " << tpi << " ns" << std::endl;

        delete[] sources_h;
        delete[] sources_d;
        delete table_d;
    }
    Kokkos::finalize();
    return 0;
}