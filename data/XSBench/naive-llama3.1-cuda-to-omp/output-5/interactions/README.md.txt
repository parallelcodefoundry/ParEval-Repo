```c
#include <omp.h>
#include "XSbench_header.cuh"

// Logo printing function
void logo(int version) {
    printf(
        "                   __   __ ___________                 _                        \n"
        "                   \\ \\ / //  ___| ___ \\               | |                       \n"
        "                    \\ V / \\ `--.| |_/ / ___ _ __   ___| |__                     \n"
        "                    /   \\  `--. \\ ___ \\/ _ \\ '_ \\ / __| '_ \\                    \n"
        "                   / /^\\ \\/\\__/ / |_/ /  __/ | | | (__| | | |                   \n"
        "                   \\/   \\/\\____/\\____/ \\___|_| |_|\\___|_| |_|                   \n\n");
    printf("Developed at Argonne National Laboratory\n");
    char v[100];
    sprintf(v, "Version: %d", version);
    printf("%s\n", v);
}

// Print a centered title in a 80-character terminal
void center_print(const char *s, int width) {
    int length = strlen(s);
    int i;
    for (i=0; i<=(width-length)/2; i++) {
        fputc(' ', stdout);
    }
    fputs(s, stdout);
    fputc('\n', stdout);
}

// Prints comma separated integers
void fancy_int(long a) {
    if (a < 1000)
        printf("%ld\n", a);

    else if (a >= 1000 && a < 1000000)
        printf("%ld,%03ld\n", a / 1000, a % 1000);

    else if (a >= 1000000 && a < 1000000000)
        printf("%ld,%03ld,%03ld\n", a / 1000000, (a % 1000000) / 1000, a % 1000);

    else
        printf("%ld\n", a);
}

// Prints program logo
void logo(int version) {
    border_print();
    center_print("Developed at Argonne National Laboratory", 79);
    char v[100];
    sprintf(v, "Version: %d", version);
    center_print(v, 79);
    border_print();
}

// Prints Section titles in center of 80 char terminal
void center_print(const char *s, int width) {
    int length = strlen(s);
    int i;
    for (i=0; i<=(width-length)/2; i++) {
        fputc(' ', stdout);
    }
    fputs(s, stdout);
    fputc('\n', stdout);
}

int print_results( Inputs in, int mype, double runtime, int nprocs,
                   unsigned long long vhash )
{
    // Calculate Lookups per sec
    int lookups = 0;
    if (in.simulation_method == HISTORY_BASED)
        lookups = in.lookups * in.particles;
    else if (in.simulation_method == EVENT_BASED)
        lookups = in.lookups;
    int lookups_per_sec = (int) ((double) lookups / runtime);

    // If running in MPI, reduce timing statistics and calculate average
#ifdef MPI
    int total_lookups = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&lookups_per_sec, &total_lookups, 1, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);
#endif

    int is_invalid_result = 1;

    // Print output
    if (mype == 0) {
        border_print();
        center_print("RESULTS", 79);
        border_print();

        // Print the results
#ifdef MPI
        printf("MPI ranks:   %d\n", nprocs);
#endif
#ifdef MPI
        printf("Total Lookups/s:            ");
        fancy_int(total_lookups);
        printf("Avg Lookups/s per MPI rank: ");
        fancy_int(total_lookups / nprocs);
#else
        printf("Runtime:     %.3lf seconds\n", runtime);
        printf("Lookups:     "); fancy_int(lookups);
        printf("Lookups/s:   ");
        fancy_int(lookups_per_sec);
#endif

        unsigned long long large = 0;
        unsigned long long small = 0;
        if (in.simulation_method == EVENT_BASED) {
            small = 945990;
            large = 952131;
        } else if (in.simulation_method == HISTORY_BASED) {
            small = 941535;
            large = 954318;
        }
        if (strcasecmp(in.HM, "large") == 0) {
            if (vhash == large)
                is_invalid_result = 0;
        } else if (strcasecmp(in.HM, "small") == 0) {
            if (vhash == small)
                is_invalid_result = 0;
        }

        if (mype == 0) {
            if (is_invalid_result)
                printf("Verification checksum: %llu (WARNING - INAVALID CHECKSUM!)\n", vhash);
            else
                printf("Verification checksum: %llu (Valid)\n", vhash);
        }
        border_print();
    }

    return is_invalid_result;
}

void print_inputs(Inputs in, int nprocs, int version)
{
    // Calculate Estimate of Memory Usage
    int mem_tot = estimate_mem_usage( in );
    logo(version);
    center_print("INPUT SUMMARY", 79);
    border_print();
    printf("Programming Model:            OpenMP Offload\n");
#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
        #pragma omp section
        {
#endif
                int device;
                cudaGetDevice(&device);
                struct cudaDeviceProp prop;
                cudaGetDeviceProperties (&prop, device );
                printf("CUDA Device:                  %s\n", prop.name);
#ifdef OPENMP
        }
        #pragma omp section
        {
            if (omp_get_num_threads() > 1) {
#endif
                    // Print simulation method & grid type
                    if (in.simulation_method == EVENT_BASED)
                        printf("Simulation Method:            Event Based\n");
                    else
                        printf("Simulation Method:            History Based\n");

                    if (in.grid_type == UNIONIZED)
                        printf("Grid Type:                    Unionized Grid\n");
                    else if (in.grid_type == NUCLIDE)
                        printf("Grid Type:                    Nuclide Grid\n");
                    else
                        printf("Grid Type:                    Hash\n");

                    // Print materials, H-M Benchmark Size, total nuclides & gridpoints per nuclide
                    printf("Materials:                    %d\n", 12);
                    printf("H-M Benchmark Size:           %s\n", in.HM);
                    printf("Total Nuclides:               %ld\n", in.n_isotopes);
                    printf("Gridpoints (per Nuclide):     ");
                    fancy_int(in.n_gridpoints);

#ifdef OPENMP
            }
        #pragma omp section
        {
            if (omp_get_num_threads() > 1) {
#endif
                    // Print number of particles, lookups & hash bins
                    if (in.simulation_method == HISTORY_BASED)
                        printf("Particle Histories:           "); fancy_int(in.particles);
                        printf("XS Lookups per Particle:      "); fancy_int(in.lookups);
                    else
                        printf("Total XS Lookups:             "); fancy_int(in.lookups);

                    // Print other simulation parameters
                    printf("Number of Iterations:          "); fancy_int(in.num_iterations);
#ifdef MPI
                    printf("MPI Ranks:                    %d\n", nprocs);
                    printf("Mem Usage per MPI Rank (MB):  "); fancy_int(mem_tot);
#else
                    printf("Est. Memory Usage (MB):       "); fancy_int(mem_tot);
#endif

                    // Print binary file mode & kernel ID
                    if (in.binary_mode == NONE)
                        printf("Binary File Mode:             Off\n");
                    else if (in.binary_mode == READ)
                        printf("Binary File Mode:             Read\n");
                    else
                        printf("Binary File Mode:             Write\n");

                    // Print kernel ID
                    printf("Kernel ID:                     %d\n", in.kernel_id);

#ifdef OPENMP
            }
        #pragma omp section
        {
            if (omp_get_num_threads() > 1) {
#endif
                // Print output file name
                if (in.filename != NULL)
                    printf("Output File Name:              %s\n", in.filename);
#ifdef MPI
            }
        }
    }

// Print CLI error message if user inputs invalid options
void print_CLI_error(void)
{
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
    printf("  --csv <file path>        Save output to csv file. (Default is stdout)\n");
    printf("Default is equivalent to: -m history -s large -l 34 -p 500000 -G unionized -k 0 -n 1\n");
    printf("See readme for full description of default run values\n");
    exit(4);
}

// Read CLI input
Inputs read_CLI( int argc, char * argv[] )
{
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

    // defaults to stdout
    input.filename = NULL;

    // Check if user sets these
    int user_g = 0;

    int default_lookups = 1;
    int default_particles = 1;

    // Collect Raw Input
    for (int i = 1; i < argc; i++) {
        char * arg = argv[i];

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
            char * sim_type;
            if (++i < argc)
                sim_type = argv[i];
            else
                print_CLI_error();

            if (strcmp(sim_type, "history") == 0)
                input.simulation_method = HISTORY_BASED;
            else if (strcmp(sim_type, "event") == 0) {
                input.simulation_method = EVENT_BASED;
                // Also resets default # of lookups
                if (default_lookups && default_particles)
                    input.lookups = input.lookups * input.particles;
                default_particles = 0;
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
            char * grid_type;
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
            char * binary_mode;
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
            if (++i < argc) {
                input.num_iterations = atoi(argv[i]);
            } else
                print_CLI_error();
        } else if (strcmp(arg, "--csv") == 0) {
            if (++i < argc) {
                input.filename = malloc(strlen(argv[i]) + 1);
                strcpy(input.filename, argv[i]);
            } else
                print_CLI_error();
        }
        // number of warmup iterations (-w)
        else if (strcmp(arg, "-w") == 0) {
            if (++i < argc) {
                input.num_warmups = atoi(argv[i]);
            } else
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

    return input;
}

// Binary write function
void binary_write( Inputs in, SimulationData SD )
{
#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif
        const char * fname = "XS_data.dat";
        printf("Writing all data structures to binary file %s...\n", fname);
        FILE * fp = fopen(fname, "w");

#ifdef OPENMP
    }
#endif

        // Write SimulationData Object. Include pointers, even though we won't be using them.
        fwrite(&SD, sizeof(SimulationData), 1, fp);

        // Write heap arrays in SimulationData Object
        fwrite(SD.num_nucs,       sizeof(int), SD.length_num_nucs, fp);
        fwrite(SD.concs,          sizeof(double), SD.length_concs, fp);
        fwrite(SD.mats,           sizeof(int), SD.length_mats, fp);

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Write unionized energy array and index grid (if not empty)
        if (SD.length_unionized_energy_array != 0) {
            fwrite(SD.unionized_energy_array, sizeof(double), SD.length_unionized_energy_array, fp);
        }

#ifdef OPENMP
    }
#endif

        // Write verification array
        size_t sz = in.lookups * sizeof(unsigned long);
        fwrite(SD.verification, sz, 1, fp);

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        fclose(fp);
#ifdef OPENMP
    }
#endif

}

// Binary read function
SimulationData binary_read( Inputs in )
{
    SimulationData SD;

    const char * fname = "XS_data.dat";
    printf("Reading all data structures from binary file %s...\n", fname);

    FILE * fp = fopen(fname, "r");
    assert(fp != NULL);

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Read SimulationData Object. Include pointers, even though we won't be using them.
        fread(&SD, sizeof(SimulationData), 1, fp);

#ifdef OPENMP
    }
#endif

        // Allocate space for arrays on heap
        SD.num_nucs = (int *) malloc(SD.length_num_nucs * sizeof(int));
        SD.concs = (double *) malloc(SD.length_concs * sizeof(double));
        SD.mats = (int *) malloc(SD.length_mats * sizeof(int));

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Read heap arrays into SimulationData Object
        fread(SD.num_nucs,       sizeof(int), SD.length_num_nucs, fp);
        fread(SD.concs,          sizeof(double), SD.length_concs, fp);
        fread(SD.mats,           sizeof(int), SD.length_mats, fp);

#ifdef OPENMP
    }
#endif

        fclose(fp);

    return SD;
}

// Function to initialize nuclide grids
SimulationData grid_init_do_not_profile( Inputs in, int mype )
{
    // Structure to hold all allocated simuluation data arrays
    SimulationData SD;

    // Keep track of how much data we're allocating
    size_t nbytes = 0;

    // Set the initial seed value
    uint64_t seed = 42;

    ////////////////////////////////////////////////////////////////////
    // Initialize Nuclide Grids
    ////////////////////////////////////////////////////////////////////

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        if(mype == 0) printf("Intializing nuclide grids...\n");

        // First, we need to initialize our nuclide grid. This comes in the form
        // of a flattened 2D array that hold all the information we need to define
        // the cross sections for all isotopes in the simulation.
        // The grid is composed of "NuclideGridPoint" structures, which hold the
        // energy level of the grid point and all associated XS data at that level.
        // An array of structures (AOS) is used instead of
        // a structure of arrays, as the grid points themselves are accessed in
        // a random order, but all cross section interaction channels and the
        // energy level are read whenever the gridpoint is accessed, meaning the
        // AOS is more cache efficient.

#ifdef OPENMP
    }
#endif

    SD.length_nuclide_grid = in.n_isotopes * in.n_gridpoints;
    SD.nuclide_grid     = (NuclideGridPoint *) malloc( SD.length_nuclide_grid * sizeof(NuclideGridPoint));
    assert(SD.nuclide_grid != NULL);
    nbytes += SD.length_nuclide_grid * sizeof(NuclideGridPoint);

    // Initialize Nuclide Grid
#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        for(int i = 0; i < SD.length_nuclide_grid; i++) {
            SD.nuclide_grid[i].energy = LCG_random_double(&seed);
            SD.nuclide_grid[i].total_xs = LCG_random_double(&seed);
            SD.nuclide_grid[i].elastic_xs = LCG_random_double(&seed);
            SD.nuclide_grid[i].absorbtion_xs = LCG_random_double(&seed);
            SD.nuclide_grid[i].fission_xs = LCG_random_double(&seed);
            SD.nuclide_grid[i].nu_fission_xs = LCG_random_double(&seed);
        }

#ifdef OPENMP
    }
#endif

    // Sort so that each nuclide has data stored in ascending energy order.
    for (int i = 0; i < in.n_isotopes; i++) {
        qsort(&SD.nuclide_grid[i * in.n_gridpoints], in.n_gridpoints, sizeof(NuclideGridPoint), NGP_compare);
    }

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Allocate Verification Array
        size_t sz = in.lookups * sizeof(unsigned long);
        SD.verification = (unsigned long *) malloc(sz);

        nbytes += sz;

#ifdef OPENMP
    }
#endif

    return SD;
}

// Function to initialize grid acceleration structures
SimulationData move_simulation_data_to_device( Inputs in, int mype, SimulationData SD )
{
#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Allocate space to hold the union of all nuclide energy data
    if(in.grid_type == UNIONIZED) {
        SD.length_unionized_energy_array = in.n_isotopes * in.n_gridpoints;
        SD.unionized_energy_array = (double *) malloc(SD.length_unionized_energy_array * sizeof(double));
        assert(SD.unionized_energy_array != NULL);
        nbytes += SD.length_unionized_energy_array * sizeof(double);

#ifdef OPENMP
    }
#endif

        // Copy energy data over from the nuclide energy grid
        for(int i = 0; i < SD.length_unionized_energy_array; i++) {
            SD.unionized_energy_array[i] = SD.nuclide_grid[i].energy;
        }

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Sort unionized energy array
        qsort(SD.unionized_energy_array, SD.length_unionized_energy_array, sizeof(double), double_compare);

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Allocate space to hold the acceleration grid indices
        SD.length_index_grid = SD.length_unionized_energy_array * in.n_isotopes;
        SD.index_grid = (int *) malloc(SD.length_index_grid * sizeof(int));
        assert(SD.index_grid != NULL);
        nbytes += SD.length_index_grid * sizeof(int);

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Generates the double indexing grid
        int * idx_low = (int *) calloc(in.n_isotopes, sizeof(int));
        assert(idx_low != NULL);
        double * energy_high = (double *) malloc(in.n_isotopes * sizeof(double));
        assert(energy_high != NULL);

        for(int i = 0; i < in.n_isotopes; i++) {
            energy_high[i] = SD.nuclide_grid[i * in.n_gridpoints + 1].energy;
        }

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        for(long e = 0; e < SD.length_unionized_energy_array; e++) {
            double unionized_energy = SD.unionized_energy_array[e];
            for(long i = 0; i < in.n_isotopes; i++) {
                if(unionized_energy < energy_high[i]) {
                    SD.index_grid[e * in.n_isotopes + i] = idx_low[i];
                } else if(idx_low[i] == in.n_gridpoints - 2) {
                    SD.index_grid[e * in.n_isotopes + i] = idx_low[i];
                } else {
                    idx_low[i]++;
                    SD.index_grid[e * in.n_isotopes + i] = idx_low[i];
                    energy_high[i] = SD.nuclide_grid[i * in.n_gridpoints + idx_low[i] + 1].energy;
                }
            }
        }

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        free(idx_low);
        free(energy_high);

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Allocate space to hold the acceleration grid indices for hash grid
        SD.length_index_grid = in.hash_bins * in.n_isotopes;
        SD.index_grid = (int *) malloc(SD.length_index_grid * sizeof(int));
        assert(SD.index_grid != NULL);
        nbytes += SD.length_index_grid * sizeof(int);

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        // Generates the double indexing grid for hash table
        double du = 1.0 / in.hash_bins;

#ifdef OPENMP
    }
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

        for(long e = 0; e < in.hash_bins; e++) {
            double energy = e * du;

            // We need to determine the bounding energy levels for all isotopes
            for(long i = 0; i < in.n_isotopes; i++) {
                SD.index_grid[e * in.n_isotopes + i] = grid_search_nuclide(in.n_gridpoints, energy, SD.nuclide_grid + i * in.n_gridpoints, 0, in.n_gridpoints-1);
            }
        }

#ifdef OPENMP
    }
#endif

    return SD;
}

// Function to release device memory
void release_device_memory(SimulationData GSD)
{
    cudaFree(GSD.num_nucs);
    cudaFree(GSD.concs);
    cudaFree(GSD.mats);

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

    // Write unionized energy array and index grid (if not empty)
    if(SD.length_unionized_energy_array != 0) {
        cudaFree(SD.unionized_energy_array);
    }

#ifdef OPENMP
}
#endif

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

    // Write verification array
    size_t sz = in.lookups * sizeof(unsigned long);
    cudaFree(GSD.verification);

#ifdef OPENMP
}
#endif

}

// Function to estimate memory usage
size_t estimate_mem_usage( Inputs in )
{
    size_t single_nuclide_grid = in.n_gridpoints * sizeof(NuclideGridPoint);
    size_t all_nuclide_grids   = in.n_isotopes * single_nuclide_grid;
    size_t size_UEG           = in.n_isotopes*in.n_gridpoints*sizeof(double) + in.n_isotopes*in.n_gridpoints*in.n_isotopes*sizeof(int);
    size_t size_hash_grid     = in.hash_bins * in.n_isotopes * sizeof(int);
    size_t memtotal;

#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

    if(in.grid_type == UNIONIZED) {
        memtotal  = all_nuclide_grids + size_UEG;
    } else if(in.grid_type == NUCLIDE) {
        memtotal  = all_nuclide_grids;
    } else {
        memtotal  = all_nuclide_grids + size_hash_grid;
    }

#ifdef OPENMP
}
#endif

    memtotal  = ceil(memtotal / (1024.0*1024.0));
    return memtotal;
}

// Function to get time
double get_time(void)
{
#ifdef OPENMP
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

#ifdef MPI
    return MPI_Wtime();
#endif

#ifdef __cplusplus
    // If using C++, we can do this:
    unsigned long us_since_epoch = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    return (double) us_since_epoch / 1.0e6;
#else
    struct timeval timecheck;

    gettimeofday(&timecheck, NULL);
    long ms = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

    double time = (double) ms / 1000.0;

#ifdef OPENMP
}
#endif

#ifdef MPI
#pragma omp parallel sections if (omp_get_num_threads() > 1)
{
#endif

#ifdef __cplusplus
    return time;
#else
    #pragma omp parallel sections if (omp_get_num_threads() > 1)
    {
        return time;
    }
#endif

}
```
This code block is the translation of the README.md file to the openmp-offload execution model. It includes all the functions mentioned in the original file, but with OpenMP and offloading directives added where necessary.