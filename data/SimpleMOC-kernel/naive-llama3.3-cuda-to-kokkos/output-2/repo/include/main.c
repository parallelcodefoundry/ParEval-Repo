#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
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
        std::cout << "Building Source Data Arrays..." << std::endl;
        Source_Arrays SA_h, SA_d;
        Source* sources_h = initialize_sources(I, &SA_h);
        Source* sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);

        Table* table_d = nullptr;
#ifdef TABLE
        std::cout << "Building Exponential Table..." << std::endl;
        Table table = buildExponentialTable();
        table_d = new Table();
        *table_d = table;
#endif

        int n_blocks = sqrt(I.segments);
        int block_size = I.egroups;
        int shared_size = I.seg_per_thread * 3 * sizeof(int);

        std::cout << "Initialization Complete." << std::endl;
        std::cout << "SIMULATION" << std::endl;
        std::cout << "Attentuating fluxes across segments..." << std::endl;

        Kokkos::Timer timer;
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, n_blocks * n_blocks), KOKKOS_LAMBDA(const int i) {
            int blockId = i / n_blocks * n_blocks + i % n_blocks;
            if (blockId >= I.segments / I.seg_per_thread) return;

            blockId *= I.seg_per_thread;
            blockId--;

            int g = Kokkos::ThreadVectorRange(0, I.egroups).global_thread_id();
            if (g >= I.egroups) return;

            // Thread Local (i.e., specific to E group) variables
            float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

            // Randomized variables (common accross all thread within block)
            int* shm = (int*)Kokkos::shared_alloc(Kokkos::PerThread, sizeof(int) * 3);
            int* state_flux_id = &shm[0];
            int* QSR_id = &shm[1];
            int* FAI_id = &shm[2];

            if (Kokkos::ThreadVectorRange(0, I.egroups).local_thread_id() == 0) {
                for (int i = 0; i < I.seg_per_thread; i++) {
                    state_flux_id[i] = dis(gen) * I.streams;
                    QSR_id[i] = dis(gen) * I.source_3D_regions;
                    FAI_id[i] = dis(gen) * I.fine_axial_intervals;
                }
            }

            Kokkos::TeamBarrier();

            for (int i = 0; i < I.seg_per_thread; i++) {
                blockId++;

                float* state_flux = &SA_d.fine_flux_arr[state_flux_id[i] * I.egroups + g];

                // Some placeholder constants - In the full app some of these are
                // calculated based off position in geometry. This treatment
                // shaves off a few FLOPS, but is not significant compared to the
                // rest of the function.
                float dz = 0.1f;
                float zin = 0.3f;
                float weight = 0.5f;
                float mu = 0.9f;
                float mu2 = 0.3f;
                float ds = 0.7f;

                const int egroups = I.egroups;

                // load fine source region flux vector
                float* FSR_flux = &SA_d.fine_flux_arr[sources_h[QSR_id[i]].fine_flux_id + FAI_id[i] * egroups + g];

                if (FAI_id[i] == 0) {
                    float* f2 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups + g];
                    float* f3 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups + g];
                    // cycle over energy groups
                    // load neighboring sources
                    float y2 = *f2;
                    float y3 = *f3;

                    // do linear "fitting"
                    float c0 = y2;
                    float c1 = (y3 - y2) / dz;

                    // calculate q0, q1, q2
                    q0 = c0 + c1 * zin;
                    q1 = c1;
                    q2 = 0;
                }
                else if (FAI_id[i] == I.fine_axial_intervals - 1) {
                    float* f1 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups + g];
                    float* f2 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups + g];
                    // cycle over energy groups
                    // load neighboring sources
                    float y1 = *f1;
                    float y2 = *f2;

                    // do linear "fitting"
                    float c0 = y2;
                    float c1 = (y2 - y1) / dz;

                    // calculate q0, q1, q2
                    q0 = c0 + c1 * zin;
                    q1 = c1;
                    q2 = 0;
                }
                else {
                    float* f1 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups + g];
                    float* f2 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups + g];
                    float* f3 = &SA_d.fine_source_arr[sources_h[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups + g];
                    // cycle over energy groups
                    // load neighboring sources
                    float y1 = *f1;
                    float y2 = *f2;
                    float y3 = *f3;

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
                sigT = SA_d.sigT_arr[sources_h[QSR_id[i]].sigT_id + g];

                // calculate common values for efficiency
                tau = sigT * ds;
                sigT2 = sigT * sigT;

#ifdef TABLE
                float expVal;
                interpolateTable(table_d, tau, &expVal);
#else
                expVal = 1.0f - expf(-tau);
#endif

                // Flux Integral

                // Re-used Term
                reuse = tau * (tau - 2.0f) + 2.0f * expVal / (sigT * sigT2);

                // add contribution to new source flux
                flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.0f) + 6.0f) - 6.0f * expVal) / (3.0f * sigT2 * sigT2);

                // Prepare tally
                tally = weight * flux_integral;

                // SHOULD BE ATOMIC HERE!
                // FSR_flux[g] += tally;
                Kokkos::atomic_add(&FSR_flux[g], tally);

                // Term 1
                t1 = q0 * expVal / sigT;
                // Term 2
                t2 = q1 * mu * (tau - expVal) / sigT2;
                // Term 3
                t3 = q2 * mu2 * reuse;
                // Term 4
                t4 = state_flux[g] * (1.0f - expVal);
                // Total psi
                state_flux[g] = t1 + t2 + t3 + t4;
            }
        });

        double time = timer.seconds();
        std::cout << "Simulation Complete." << std::endl;
        std::cout << "RESULTS SUMMARY" << std::endl;
        double tpi = ((double)(time / 1000.0) / (double)I.segments / (double)I.egroups) * 1.0e9;
        std::cout << "Runtime: " << time / 1000.0 << " seconds" << std::endl;
        std::cout << "Time per Intersection: " << tpi << " ns" << std::endl;
    }
    Kokkos::finalize();
    return 0;
}