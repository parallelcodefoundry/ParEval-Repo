#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include "XSbench_shared_header.h"
#include "XSbench_header.h"

// Function to print program logo
void logo(int version) {
    printf(
        "                   __   __ ___________                 _                        \n"
        "                   \\ \\ / //  ___| ___ \\               | |                       \n"
        "                    \\ V / \\ `--.| |_/ / ___ _ __   ___| |__                     \n"
        "                    /   \\  `--. \\ ___ \\/ _ \\ '_ \\ / __| '_ \\                    \n"
        "                   / /^\\ \\/\\__/ / |_/ /  __/ | | | (__| | | |                   \n"
        "                   \\/   \\/\\____/\\____/ \\___|_| |_|\\___|_| |_|                   \n\n"
    );
    printf("Developed at Argonne National Laboratory\n");
    char v[100];
    sprintf(v, "Version: %d", version);
    printf("%s\n", v);
}

// Function to print section titles in center of 80 char terminal
void center_print(const char *s, int width) {
    int length = strlen(s);
    int i;
    for (i = 0; i <= (width - length) / 2; i++) {
        printf(" ");
    }
    printf("%s\n", s);
}

// Function to print comma separated integers
void fancy_int(long a) {
    if (a < 1000) {
        printf("%ld\n", a);
    } else if (a >= 1000 && a < 1000000) {
        printf("%ld,%03ld\n", a / 1000, a % 1000);
    } else if (a >= 1000000 && a < 1000000000) {
        printf("%ld,%03ld,%03ld\n", a / 1000000, (a % 1000000) / 1000, a % 1000);
    } else if (a >= 1000000000) {
        printf("%ld,%03ld,%03ld,%03ld\n", a / 1000000000, (a % 1000000000) / 1000000, (a % 1000000) / 1000, a % 1000);
    } else {
        printf("%ld\n", a);
    }
}

// Function to print CLI error
void print_CLI_error(void) {
    printf("Usage: ./XSBench <options>\n");
    printf("Options include:\n");
    printf("  -m <simulation method>   Simulation method (history, event)\n");
    printf("  -s <size>                Size of H-M Benchmark to run (small, large, XL, XXL)\n");
    printf("  -g <gridpoints>          Number of gridpoints per nuclide (overrides -s defaults)\n");
    printf("  -G <grid type>           Grid search type (unionized, nuclide, hash). Defaults to unionized.\n");
    printf("  -p <particles>           Number of particle histories\n");
    printf("  -l <lookups>             History Based: Number of Cross-section (XS) lookups per particle. Event Based: Total number of XS lookups.\n");
    printf("  -h <hash bins>           Number of hash bins (only relevant when used with \"-G hash\")\n");
    printf("  -b <binary mode>         Read or write all data structures to file. If reading, this will skip initialization phase. (read, write)\n");
    printf("  -k <kernel ID>           Specifies which kernel to run. 0 is baseline, 1, 2, etc are optimized variants. (0 is default.)\n");
    printf("  -n <num iterations>      Specifies how many kernel iterations to run. (1 is default.)\n");
    printf("  -w <num warmups>         Specifies how many warmup iterations to run. (0 is default.)\n");
    printf("  --csv <file path>        Save output to csv file. (Default is stdout)\n");
    exit(4);
}

// Function to read CLI
Inputs read_CLI(int argc, char *argv[]) {
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

    // Check if user sets these
    int user_g = 0;

    int default_lookups = 1;
    int default_particles = 1;

    // Collect Raw Input
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        // n_gridpoints (-g)
        if (strcmp(arg, "-g") == 0) {
            if (++i < argc) {
                user_g = 1;
                input.n_gridpoints = atol(argv[i]);
            } else
                print_CLI_error();
        }
        // Simulation Method (-m)
        else if (strcmp(arg, "-m") == 0) {
            char *sim_type;
            if (++i < argc)
                sim_type = argv[i];
            else
                print_CLI_error();

            if (strcmp(sim_type, "history") == 0)
                input.simulation_method = HISTORY_BASED;
            else if (strcmp(sim_type, "event") == 0) {
                input.simulation_method = EVENT_BASED;
                // Also resets default # of lookups
                if (default_lookups && default_particles) {
                    input.lookups = input.lookups * input.particles;
                    input.particles = 0;
                }
            } else
                print_CLI_error();
        }
        // lookups (-l)
        else if (strcmp(arg, "-l") == 0) {
            if (++i < argc) {
                input.lookups = atoi(argv[i]);
                default_lookups = 0;
            } else
                print_CLI_error();
        }
        // hash bins (-h)
        else if (strcmp(arg, "-h") == 0) {
            if (++i < argc)
                input.hash_bins = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // particles (-p)
        else if (strcmp(arg, "-p") == 0) {
            if (++i < argc) {
                input.particles = atoi(argv[i]);
                default_particles = 0;
            } else
                print_CLI_error();
        }
        // HM (-s)
        else if (strcmp(arg, "-s") == 0) {
            if (++i < argc)
                input.HM = argv[i];
            else
                print_CLI_error();
        }
        // grid type (-G)
        else if (strcmp(arg, "-G") == 0) {
            char *grid_type;
            if (++i < argc)
                grid_type = argv[i];
            else
                print_CLI_error();

            if (strcmp(grid_type, "unionized") == 0)
                input.grid_type = UNIONIZED;
            else if (strcmp(grid_type, "nuclide") == 0)
                input.grid_type = NUCLIDE;
            else if (strcmp(grid_type, "hash") == 0)
                input.grid_type = HASH;
            else
                print_CLI_error();
        }
        // binary mode (-b)
        else if (strcmp(arg, "-b") == 0) {
            char *binary_mode;
            if (++i < argc)
                binary_mode = argv[i];
            else
                print_CLI_error();

            if (strcmp(binary_mode, "read") == 0)
                input.binary_mode = READ;
            else if (strcmp(binary_mode, "write") == 0)
                input.binary_mode = WRITE;
            else
                print_CLI_error();
        }
        // kernel optimization selection (-k)
        else if (strcmp(arg, "-k") == 0) {
            if (++i < argc) {
                input.kernel_id = atoi(argv[i]);
            } else
                print_CLI_error();
        }
        // number of kernel iterations (-n)
        else if (strcmp(arg, "-n") == 0) {
            if (++i < argc)
                input.num_iterations = atoi(argv[i]);
            else
                print_CLI_error();
        }
        else if (strcmp(arg, "--csv") == 0) {
            if (++i < argc) {
                input.filename = (char *)malloc(strlen(argv[i]) + 1);
                strcpy(input.filename, argv[i]);
            } else
                print_CLI_error();
        }
        else if (strcmp(arg, "-w") == 0) {
            if (++i < argc)
                input.num_warmups = atoi(argv[i]);
            else
                print_CLI_error();
        }
        else
            print_CLI_error();
    }

    // Validate Input

    // Validate nthreads
    if (input.nthreads < 1)
        print_CLI_error();

    // Validate n_isotopes
    if (input.n_isotopes < 1)
        print_CLI_error();

    // Validate n_gridpoints
    if (input.n_gridpoints < 1)
        print_CLI_error();

    // Validate lookups
    if (input.lookups < 1)
        print_CLI_error();

    // Validate Hash Bins
    if (input.hash_bins < 1)
        print_CLI_error();

    // Validate number of iterations
    if (input.num_iterations < 1)
        print_CLI_error();

    // Validate HM size
    if (strcasecmp(input.HM, "small") != 0 &&
        strcasecmp(input.HM, "large") != 0 &&
        strcasecmp(input.HM, "XL") != 0 &&
        strcasecmp(input.HM, "XXL") != 0)
        print_CLI_error();

    // Set HM size specific parameters
    // (defaults to large)
    if (strcasecmp(input.HM, "small") == 0)
        input.n_isotopes = 68;
    else if (strcasecmp(input.HM, "XL") == 0 && user_g == 0)
        input.n_gridpoints = 238847; // sized to make 120 GB XS data
    else if (strcasecmp(input.HM, "XXL") == 0 && user_g == 0)
        input.n_gridpoints = 238847 * 2.1; // 252 GB XS data

    // Return input struct
    return input;
}

// Function to print inputs
void print_inputs(Inputs in, int nprocs, int version) {
    // Calculate Estimate of Memory Usage
    int mem_tot = estimate_mem_usage(in);
    logo(version);
    center_print("INPUT SUMMARY", 79);
    printf("Programming Model:            OpenMP-Offload\n");
    printf("Simulation Method:            ");
    if (in.simulation_method == EVENT_BASED)
        printf("Event Based\n");
    else
        printf("History Based\n");
    printf("Grid Type:                    ");
    if (in.grid_type == NUCLIDE)
        printf("Nuclide Grid\n");
    else if (in.grid_type == UNIONIZED)
        printf("Unionized Grid\n");
    else
        printf("Hash\n");
    printf("Materials:                    12\n");
    printf("H-M Benchmark Size:           %s\n", in.HM);
    printf("Total Nuclides:               %ld\n", in.n_isotopes);
    printf("Gridpoints (per Nuclide):     ");
    fancy_int(in.n_gridpoints);
    if (in.grid_type == HASH) {
        printf("Hash Bins:                    ");
        fancy_int(in.hash_bins);
    }
    if (in.grid_type == UNIONIZED) {
        printf("Unionized Energy Gridpoints:  ");
        fancy_int(in.n_isotopes * in.n_gridpoints);
    }
    if (in.simulation_method == HISTORY_BASED) {
        printf("Particle Histories:           ");
        fancy_int(in.particles);
        printf("XS Lookups per Particle:      ");
        fancy_int(in.lookups);
    }
    printf("Total XS Lookups:             ");
    fancy_int(in.lookups);
    printf("Total XS Iterations:          ");
    fancy_int(in.num_iterations);
    printf("Binary File Mode:             ");
    if (in.binary_mode == NONE)
        printf("Off\n");
    else if (in.binary_mode == READ)
        printf("Read\n");
    else
        printf("Write\n");
    printf("Mem Usage per MPI Rank (MB):  ");
    fancy_int(mem_tot);
    printf("\n");
}

// Function to print results
int print_results(Inputs in, int mype, double runtime, int nprocs, unsigned long long vhash) {
    // Calculate Lookups per sec
    int lookups = 0;
    if (in.simulation_method == HISTORY_BASED)
        lookups = in.lookups * in.particles;
    else if (in.simulation_method == EVENT_BASED)
        lookups = in.lookups;
    int lookups_per_sec = (int)((double)lookups / runtime);

    // If running in MPI, reduce timing statistics and calculate average
    int total_lookups = 0;
    #ifdef MPI
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&lookups_per_sec, &total_lookups, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    #endif

    int is_invalid_result = 1;

    // Print output
    if (mype == 0) {
        printf("\n");
        center_print("RESULTS", 79);
        printf("\n");
        // Print the results
        printf("NOTE: Timings are estimated -- use nvprof/nsys/iprof/rocprof for formal analysis\n");
        #ifdef MPI
        printf("MPI ranks:   %d\n", nprocs);
        printf("Total Lookups/s:            ");
        fancy_int(total_lookups);
        printf("Avg Lookups/s per MPI rank: ");
        fancy_int(total_lookups / nprocs);
        #else
        printf("Runtime:     %.3lf seconds\n", runtime);
        printf("Lookups:     ");
        fancy_int(lookups);
        printf("Lookups/s:   ");
        fancy_int(lookups_per_sec);
        #endif
    }

    unsigned long long large = 0;
    unsigned long long small = 0;
    if (in.simulation_method == EVENT_BASED) {
        small = 945990;
        large = 952131;
    } else if (in.simulation_method == HISTORY_BASED) {
        small = 941535;
        large = 954318;
    }
    if (strcmp(in.HM, "large") == 0) {
        if (vhash == large)
            is_invalid_result = 0;
    } else if (strcmp(in.HM, "small") == 0) {
        if (vhash == small)
            is_invalid_result = 0;
    }

    if (mype == 0) {
        if (is_invalid_result)
            printf("Verification checksum: %llu (WARNING - INVALID CHECKSUM!)\n", vhash);
        else
            printf("Verification checksum: %llu (Valid)\n", vhash);
    }

    return is_invalid_result;
}

// Function to print profile
void print_profile(Profile profile, Inputs in) {
    if (in.filename) {
        FILE *output = fopen(in.filename, "w");
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

int main(int argc, char *argv[]) {
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
    if (mype == 0)
        print_inputs(in, nprocs, version);

    // =====================================================================
    // Prepare Nuclide Energy Grids, Unionized Energy Grid, & Material Data
    // This is not reflective of a real Monte Carlo simulation workload,
    // therefore, do not profile this region!
    // =====================================================================

    SimulationData SD;

    // If read from file mode is selected, skip initialization and load
    // all simulation data structures from file instead
    if (in.binary_mode == READ)
        SD = binary_read(in);
    else
        SD = grid_init_do_not_profile(in, mype);

    // If writing from file mode is selected, write all simulation data
    // structures to file
    if (in.binary_mode == WRITE && mype == 0)
        binary_write(in, SD);

    Profile profile;

    // =====================================================================
    // Cross Section (XS) Parallel Lookup Simulation
    // This is the section that should be profiled, as it reflects a
    // realistic continuous energy Monte Carlo macroscopic cross section
    // lookup kernel.
    // =====================================================================
    if (mype == 0) {
        printf("\n");
        center_print("SIMULATION", 79);
        printf("\n");
    }

    // Start Simulation Timer
    omp_start = get_time();

    // Run simulation
    if (in.simulation_method == EVENT_BASED) {
        if (in.kernel_id == 0)
            verification = run_event_based_simulation_baseline(in, SD, mype, &profile);
        else if (in.kernel_id == 1)
            verification = run_event_based_simulation_optimization_1(in, SD, mype);
        else if (in.kernel_id == 2)
            verification = run_event_based_simulation_optimization_2(in, SD, mype);
        else if (in.kernel_id == 3)
            verification = run_event_based_simulation_optimization_3(in, SD, mype);
        else if (in.kernel_id == 4)
            verification = run_event_based_simulation_optimization_4(in, SD, mype);
        else if (in.kernel_id == 5)
            verification = run_event_based_simulation_optimization_5(in, SD, mype);
        else if (in.kernel_id == 6)
            verification = run_event_based_simulation_optimization_6(in, SD, mype);
        else {
            printf("Error: No kernel ID %d found!\n", in.kernel_id);
            exit(1);
        }
    } else {
        printf(
            "History-based simulation not implemented in OpenMP-Offload code. Instead,\nuse "
            "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    if (mype == 0) {
        printf("\n");
        printf("Simulation complete.\n");
    }

    // End Simulation Timer
    omp_end = get_time();

    // Release device memory
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
        print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}