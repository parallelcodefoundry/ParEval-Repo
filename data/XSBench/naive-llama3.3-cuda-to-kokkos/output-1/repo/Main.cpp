#include "XSbench_header.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

// Function to print logo
void logo(int version) {
    std::cout << "                   __   __ ___________                 _                        \n"
              << "                   \\ \\ / //  ___| ___ \\               | |                       \n"
              << "                    \\ V / \\ `--.| |_/ / ___ _ __   ___| |__                     \n"
              << "                    /   \\  `--. \\ ___ \\/ _ \\ '_ \\ / __| '_ \\                    \n"
              << "                   / /^\\ \\/\\__/ / |_/ /  __/ | | | (__| | | |                   \n"
              << "                   \\/   \\/\\____/\\____/ \\___|_| |_|\\___|_| |_|                   \n\n"
              << "===============================================================================\n"
              << "Developed at Argonne National Laboratory\n"
              << "Version: " << version << "\n"
              << "===============================================================================\n";
}

// Function to print centered text
void center_print(const std::string& s, int width) {
    int length = s.length();
    int i;
    for (i = 0; i <= (width - length) / 2; i++) {
        std::cout << " ";
    }
    std::cout << s << "\n";
}

// Function to print border
void border_print() {
    std::cout << "===============================================================================\n";
}

// Function to print comma separated integers
void fancy_int(long a) {
    if (a < 1000) {
        std::cout << a << "\n";
    } else if (a >= 1000 && a < 1000000) {
        std::cout << a / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << "\n";
    } else if (a >= 1000000 && a < 1000000000) {
        std::cout << a / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << "\n";
    } else if (a >= 1000000000) {
        std::cout << a / 1000000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000000) / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << "\n";
    } else {
        std::cout << a << "\n";
    }
}

// Function to print CLI error
void print_CLI_error() {
    std::cout << "Usage: ./XSBench <options>\n";
    std::cout << "Options include:\n";
    std::cout << "  -m <simulation method>   Simulation method (history, event)\n";
    std::cout << "  -s <size>                Size of H-M Benchmark to run (small, large, XL, XXL)\n";
    std::cout << "  -g <gridpoints>          Number of gridpoints per nuclide (overrides -s defaults)\n";
    std::cout << "  -G <grid type>           Grid search type (unionized, nuclide, hash). Defaults to unionized.\n";
    std::cout << "  -p <particles>           Number of particle histories\n";
    std::cout << "  -l <lookups>             History Based: Number of Cross-section (XS) lookups per particle. Event Based: Total number of XS lookups.\n";
    std::cout << "  -h <hash bins>           Number of hash bins (only relevant when used with \"-G hash\")\n";
    std::cout << "  -b <binary mode>         Read or write all data structures to file. If reading, this will skip initialization phase. (read, write)\n";
    std::cout << "  -k <kernel ID>           Specifies which kernel to run. 0 is baseline, 1, 2, etc are optimized variants. (0 is default.)\n";
    std::cout << "  -n <num iterations>      Specifies how many kernel iterations to run. (1 is default.)\n";
    std::cout << "  -w <num warmups>         Specifies how many warmup iterations to run. (0 is default.)\n";
    std::cout << "  --csv <file path>        Save output to csv file. (Default is stdout)\n";
    std::cout << "Default is equivalent to: -m history -s large -l 34 -p 500000 -G unionized -k 0 -n 1\n";
    std::cout << "See readme for full description of default run values\n";
    exit(4);
}

// Function to read CLI
Inputs read_CLI(int argc, char* argv[]) {
    Inputs input;

    // defaults to the history based simulation method
    input.simulation_method = HISTORY_BASED;

    // defaults to max threads on the system
    input.nthreads = 1;

    // defaults to 355 (corresponding to H-M Large benchmark)
    input.n_isotopes = 355;

    // defaults to 11303 (corresponding to H-M Large benchmark)
    input.n_gridpoints = 11303;

    // defaults to 500,000
    input.particles = 500000;

    // defaults to 34
    input.lookups = 34;

    // default to unionized grid
    input.grid_type = UNIONIZED;

    // default to unionized grid
    input.hash_bins = 10000;

    // default to no binary read/write
    input.binary_mode = NONE;

    // defaults to baseline kernel
    input.kernel_id = 0;

    // default to one kernel iteration
    input.num_iterations = 1;

    // default to zero warmup iterations
    input.num_warmups = 1;

    // default to stdout
    input.filename = NULL;

    // defaults to H-M Large benchmark
    input.HM = (char*)malloc(6 * sizeof(char));
    input.HM[0] = 'l';
    input.HM[1] = 'a';
    input.HM[2] = 'r';
    input.HM[3] = 'g';
    input.HM[4] = 'e';
    input.HM[5] = '\0';

    // Check if user sets these
    int user_g = 0;

    int default_lookups = 1;
    int default_particles = 1;

    // Collect Raw Input
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // n_gridpoints (-g)
        if (arg == "-g") {
            if (++i < argc) {
                user_g = 1;
                input.n_gridpoints = std::stol(argv[i]);
            } else {
                print_CLI_error();
            }
        }
        // Simulation Method (-m)
        else if (arg == "-m") {
            std::string sim_type;
            if (++i < argc) {
                sim_type = argv[i];
            } else {
                print_CLI_error();
            }

            if (sim_type == "history") {
                input.simulation_method = HISTORY_BASED;
            } else if (sim_type == "event") {
                input.simulation_method = EVENT_BASED;
                // Also resets default # of lookups
                if (default_lookups && default_particles) {
                    input.lookups = input.lookups * input.particles;
                    input.particles = 0;
                }
            } else {
                print_CLI_error();
            }
        }
        // lookups (-l)
        else if (arg == "-l") {
            if (++i < argc) {
                input.lookups = std::stoi(argv[i]);
                default_lookups = 0;
            } else {
                print_CLI_error();
            }
        }
        // hash bins (-h)
        else if (arg == "-h") {
            if (++i < argc) {
                input.hash_bins = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }
        // particles (-p)
        else if (arg == "-p") {
            if (++i < argc) {
                input.particles = std::stoi(argv[i]);
                default_particles = 0;
            } else {
                print_CLI_error();
            }
        }
        // HM (-s)
        else if (arg == "-s") {
            if (++i < argc) {
                input.HM = argv[i];
            } else {
                print_CLI_error();
            }
        }
        // grid type (-G)
        else if (arg == "-G") {
            std::string grid_type;
            if (++i < argc) {
                grid_type = argv[i];
            } else {
                print_CLI_error();
            }

            if (grid_type == "unionized") {
                input.grid_type = UNIONIZED;
            } else if (grid_type == "nuclide") {
                input.grid_type = NUCLIDE;
            } else if (grid_type == "hash") {
                input.grid_type = HASH;
            } else {
                print_CLI_error();
            }
        }
        // binary mode (-b)
        else if (arg == "-b") {
            std::string binary_mode;
            if (++i < argc) {
                binary_mode = argv[i];
            } else {
                print_CLI_error();
            }

            if (binary_mode == "read") {
                input.binary_mode = READ;
            } else if (binary_mode == "write") {
                input.binary_mode = WRITE;
            } else {
                print_CLI_error();
            }
        }
        // kernel optimization selection (-k)
        else if (arg == "-k") {
            if (++i < argc) {
                input.kernel_id = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }
        // number of kernel iterations (-n)
        else if (arg == "-n") {
            if (++i < argc) {
                input.num_iterations = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }
        // number of warmup iterations (-w)
        else if (arg == "-w") {
            if (++i < argc) {
                input.num_warmups = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }
        // csv output file (--csv)
        else if (arg == "--csv") {
            if (++i < argc) {
                input.filename = argv[i];
            } else {
                print_CLI_error();
            }
        } else {
            print_CLI_error();
        }
    }

    // Validate Input

    // Validate nthreads
    if (input.nthreads < 1) {
        print_CLI_error();
    }

    // Validate n_isotopes
    if (input.n_isotopes < 1) {
        print_CLI_error();
    }

    // Validate n_gridpoints
    if (input.n_gridpoints < 1) {
        print_CLI_error();
    }

    // Validate lookups
    if (input.lookups < 1) {
        print_CLI_error();
    }

    // Validate Hash Bins
    if (input.hash_bins < 1) {
        print_CLI_error();
    }

    // Validate number of iterations
    if (input.num_iterations < 1) {
        print_CLI_error();
    }

    // Validate HM size
    if (std::string(input.HM) != "small" && std::string(input.HM) != "large" && std::string(input.HM) != "XL" && std::string(input.HM) != "XXL") {
        print_CLI_error();
    }

    // Set HM size specific parameters
    // (defaults to large)
    if (std::string(input.HM) == "small") {
        input.n_isotopes = 68;
    } else if (std::string(input.HM) == "XL" && user_g == 0) {
        input.n_gridpoints = 238847; // sized to make 120 GB XS data
    } else if (std::string(input.HM) == "XXL" && user_g == 0) {
        input.n_gridpoints = 238847 * 2.1; // 252 GB XS data
    }

    // Return input struct
    return input;
}

// Function to print inputs
void print_inputs(Inputs in, int nprocs, int version) {
    // Calculate Estimate of Memory Usage
    int mem_tot = estimate_mem_usage(in);

    logo(version);
    center_print("INPUT SUMMARY", 79);
    border_print();
    std::cout << "Programming Model:            Kokkos\n";
    std::cout << "Simulation Method:            ";
    if (in.simulation_method == HISTORY_BASED) {
        std::cout << "History Based\n";
    } else {
        std::cout << "Event Based\n";
    }
    std::cout << "Grid Type:                    ";
    if (in.grid_type == UNIONIZED) {
        std::cout << "Unionized Grid\n";
    } else if (in.grid_type == NUCLIDE) {
        std::cout << "Nuclide Grid\n";
    } else {
        std::cout << "Hash Grid\n";
    }
    std::cout << "Materials:                    12\n";
    std::cout << "H-M Benchmark Size:           " << in.HM << "\n";
    std::cout << "Total Nuclides:               " << in.n_isotopes << "\n";
    std::cout << "Gridpoints (per Nuclide):     ";
    fancy_int(in.n_gridpoints);
    if (in.grid_type == HASH) {
        std::cout << "Hash Bins:                    ";
        fancy_int(in.hash_bins);
    }
    if (in.grid_type == UNIONIZED) {
        std::cout << "Unionized Energy Gridpoints:  ";
        fancy_int(in.n_isotopes * in.n_gridpoints);
    }
    if (in.simulation_method == HISTORY_BASED) {
        std::cout << "Particle Histories:           ";
        fancy_int(in.particles);
        std::cout << "XS Lookups per Particle:      ";
        fancy_int(in.lookups);
    }
    std::cout << "Total XS Lookups:             ";
    fancy_int(in.lookups);
    std::cout << "Total XS Iterations:          ";
    fancy_int(in.num_iterations);
    std::cout << "Mem Usage per MPI Rank (MB):  ";
    fancy_int(mem_tot);
    std::cout << "Binary File Mode:             ";
    if (in.binary_mode == NONE) {
        std::cout << "Off\n";
    } else if (in.binary_mode == READ) {
        std::cout << "Read\n";
    } else {
        std::cout << "Write\n";
    }
    border_print();
    center_print("INITIALIZATION - DO NOT PROFILE", 79);
    border_print();
}

// Function to print results
int print_results(Inputs in, int mype, double runtime, int nprocs, unsigned long long vhash) {
    // Calculate Lookups per sec
    int lookups = 0;
    if (in.simulation_method == HISTORY_BASED) {
        lookups = in.lookups * in.particles;
    } else if (in.simulation_method == EVENT_BASED) {
        lookups = in.lookups;
    }
    int lookups_per_sec = (int)((double)lookups / runtime);

    // If running in MPI, reduce timing statistics and calculate average
    int total_lookups = 0;
    // MPI_Barrier(MPI_COMM_WORLD);
    // MPI_Reduce(&lookups_per_sec, &total_lookups, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    int is_invalid_result = 1;

    // Print output
    if (mype == 0) {
        border_print();
        center_print("RESULTS", 79);
        border_print();

        // Print the results
        std::cout << "NOTE: Timings are estimated -- use nvprof/nsys/iprof/rocprof for formal analysis\n";
        std::cout << "MPI ranks:   " << nprocs << "\n";
        std::cout << "Total Lookups/s:            ";
        fancy_int(total_lookups);
        std::cout << "Avg Lookups/s per MPI rank: ";
        fancy_int(total_lookups / nprocs);
        std::cout << "Verification checksum: ";
        if (vhash == 945990) {
            std::cout << vhash << " (Valid)\n";
        } else {
            std::cout << vhash << " (WARNING - INVALID CHECKSUM!)\n";
        }
    }

    return is_invalid_result;
}

// Function to binary write
void binary_write(Inputs in, SimulationData SD) {
    const char* fname = "XS_data.dat";
    std::cout << "Writing all data structures to binary file " << fname << "...\n";
    std::ofstream fp(fname, std::ios::binary);

    // Write SimulationData Object. Include pointers, even though we won't be using them.
    fp.write((char*)&SD, sizeof(SimulationData));

    // Write heap arrays in SimulationData Object
    fp.write((char*)SD.num_nucs, SD.length_num_nucs * sizeof(int));
    fp.write((char*)SD.concs, SD.length_concs * sizeof(double));
    fp.write((char*)SD.mats, SD.length_mats * sizeof(int));
    fp.write((char*)SD.nuclide_grid, SD.length_nuclide_grid * sizeof(NuclideGridPoint));
    fp.write((char*)SD.index_grid, SD.length_index_grid * sizeof(int));
    fp.write((char*)SD.unionized_energy_array, SD.length_unionized_energy_array * sizeof(double));

    fp.close();
}

// Function to binary read
SimulationData binary_read(Inputs in) {
    SimulationData SD;

    const char* fname = "XS_data.dat";
    std::cout << "Reading all data structures from binary file " << fname << "...\n";

    std::ifstream fp(fname, std::ios::binary);
    assert(fp.is_open());

    // Read SimulationData Object. Include pointers, even though we won't be using them.
    fp.read((char*)&SD, sizeof(SimulationData));

    // Allocate space for arrays on heap
    SD.num_nucs = (int*)malloc(SD.length_num_nucs * sizeof(int));
    SD.concs = (double*)malloc(SD.length_concs * sizeof(double));
    SD.mats = (int*)malloc(SD.length_mats * sizeof(int));
    SD.nuclide_grid = (NuclideGridPoint*)malloc(SD.length_nuclide_grid * sizeof(NuclideGridPoint));
    SD.index_grid = (int*)malloc(SD.length_index_grid * sizeof(int));
    SD.unionized_energy_array = (double*)malloc(SD.length_unionized_energy_array * sizeof(double));

    // Read heap arrays into SimulationData Object
    fp.read((char*)SD.num_nucs, SD.length_num_nucs * sizeof(int));
    fp.read((char*)SD.concs, SD.length_concs * sizeof(double));
    fp.read((char*)SD.mats, SD.length_mats * sizeof(int));
    fp.read((char*)SD.nuclide_grid, SD.length_nuclide_grid * sizeof(NuclideGridPoint));
    fp.read((char*)SD.index_grid, SD.length_index_grid * sizeof(int));
    fp.read((char*)SD.unionized_energy_array, SD.length_unionized_energy_array * sizeof(double));

    fp.close();

    return SD;
}

int main(int argc, char* argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0;
    double omp_start, omp_end;
    int nprocs = 1;
    unsigned long long verification;

    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI(argc, argv);

    // Print-out of Input Summary
    if (mype == 0) {
        print_inputs(in, nprocs, version);
    }

    // =====================================================================
    // Prepare Nuclide Energy Grids, Unionized Energy Grid, & Material Data
    // This is not reflective of a real Monte Carlo simulation workload,
    // therefore, do not profile this region!
    // =====================================================================

    SimulationData SD;

    // If read from file mode is selected, skip initialization and load
    // all simulation data structures from file instead
    if (in.binary_mode == READ) {
        SD = binary_read(in);
    } else {
        SD = grid_init_do_not_profile(in, mype);
    }

    // If writing from file mode is selected, write all simulation data
    // structures to file
    if (in.binary_mode == WRITE && mype == 0) {
        binary_write(in, SD);
    }

    Profile profile;

    // =====================================================================
    // Cross Section (XS) Parallel Lookup Simulation
    // This is the section that should be profiled, as it reflects a
    // realistic continuous energy Monte Carlo macroscopic cross section
    // lookup kernel.
    // =====================================================================
    if (mype == 0) {
        std::cout << "\n";
        border_print();
        center_print("SIMULATION", 79);
        border_print();
    }

    // Start Simulation Timer
    omp_start = get_time();

    // Run simulation
    if (in.simulation_method == EVENT_BASED) {
        if (in.kernel_id == 0) {
            verification = run_event_based_simulation_baseline(in, SD, mype, &profile);
        } else if (in.kernel_id == 1) {
            verification = run_event_based_simulation_optimization_1(in, SD, mype);
        } else if (in.kernel_id == 2) {
            verification = run_event_based_simulation_optimization_2(in, SD, mype);
        } else if (in.kernel_id == 3) {
            verification = run_event_based_simulation_optimization_3(in, SD, mype);
        } else if (in.kernel_id == 4) {
            verification = run_event_based_simulation_optimization_4(in, SD, mype);
        } else if (in.kernel_id == 5) {
            verification = run_event_based_simulation_optimization_5(in, SD, mype);
        } else if (in.kernel_id == 6) {
            verification = run_event_based_simulation_optimization_6(in, SD, mype);
        } else {
            std::cout << "Error: No kernel ID " << in.kernel_id << " found!\n";
            exit(1);
        }
    } else {
        std::cout << "History-based simulation not implemented in Kokkos code. Instead, use the event-based method with \"-m event\" argument.\n";
        exit(1);
    }

    if (mype == 0) {
        std::cout << "\n";
        std::cout << "Simulation complete.\n";
    }

    // End Simulation Timer
    omp_end = get_time();

    // Release device memory
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result = print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}