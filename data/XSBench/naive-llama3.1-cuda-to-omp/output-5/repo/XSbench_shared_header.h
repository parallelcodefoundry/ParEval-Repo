#ifndef XSBENCH_SHARED_HEADER_H
#define XSBENCH_SHARED_HEADER_H

#include <omp.h>
#include <cuda_runtime.h>

// Header for shared utilities across XSBench versions

typedef struct{
        int nthreads;
        long n_isotopes;
        long n_gridpoints;
        int lookups;
        char * HM;
        int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
        int hash_bins;
        int particles;
        int simulation_method;
        int binary_mode;
        int kernel_id;
        int num_iterations;
        int num_warmups;
        char *filename;
} Inputs;

typedef struct{
  double device_to_host_time;
  double kernel_time;
  double host_to_device_time;
} Profile;

// Offload declarations for the OpenMP-Offload model
#ifdef __CUDA_OFFLOAD__
__host__ void print_profile(Profile profile, Inputs in) {
  if (in.filename) {
    FILE* output = fopen(in.filename, "w");
    fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    fprintf(output, "%f,%f,%f,%d,%d\n",
            profile.host_to_device_time*1000,
            profile.kernel_time*1000,
            profile.device_to_host_time*1000,
            in.num_iterations,
            in.num_warmups);
    fclose(output);
  }
  else {
    printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    printf("%f,%f,%f,%d,%d\n",
           profile.host_to_device_time*1000,
           profile.kernel_time*1000,
           profile.device_to_host_time*1000,
           in.num_iterations,
           in.num_warmups);
  }
}

__host__ void print_CLI_error(void) {
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
    printf("Default is equivalent to: -m history -s large -l 34 -p 500000 -G unionized -k 0 -n 1\n");
    printf("See readme for full description of default run values\n");
    exit(4);
}

__host__ Inputs read_CLI( int argc, char * argv[] ) {
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
        for( int i = 1; i < argc; i++ )
        {
                char * arg = argv[i];

                // n_gridpoints (-g)
                if( strcmp(arg, "-g") == 0 )
                {
                        if( ++i < argc )
                        {
                                user_g = 1;
                                input.n_gridpoints = atol(argv[i]);
                        }
                        else
                                print_CLI_error();
                }
                // Simulation Method (-m)
                else if( strcmp(arg, "-m") == 0 )
                {
                        char * sim_type;
                        if( ++i < argc )
                                sim_type = argv[i];
                        else
                                print_CLI_error();

                        if( strcmp(sim_type, "history") == 0 )
                                input.simulation_method = HISTORY_BASED;
                        else if( strcmp(sim_type, "event") == 0 )
                {
                        input.simulation_method = EVENT_BASED;
                        // Also resets default # of lookups
                        if( default_lookups && default_particles )
                        {
                                input.lookups =  input.lookups * input.particles;
                                input.particles = 0;
                        }
                }
                else
                        print_CLI_error();
        }
                // lookups (-l)
                else if( strcmp(arg, "-l") == 0 )
                {
                        if( ++i < argc )
                        {
                                input.lookups = atoi(argv[i]);
                                default_lookups = 0;
                        }
                        else
                                print_CLI_error();
                }
                // hash bins (-h)
                else if( strcmp(arg, "-h") == 0 )
                {
                        if( ++i < argc )
                                input.hash_bins = atoi(argv[i]);
                        else
                                print_CLI_error();
                }
                // particles (-p)
                else if( strcmp(arg, "-p") == 0 )
                {
                        if( ++i < argc )
                        {
                                input.particles = atoi(argv[i]);
                                default_particles = 0;
                        }
                        else
                                print_CLI_error();
                }
                // HM (-s)
                else if( strcmp(arg, "-s") == 0 )
                {
                        if( ++i < argc )
                                input.HM = argv[i];
                        else
                                print_CLI_error();
                }
                // grid type (-G)
                else if( strcmp(arg, "-G") == 0 )
                {
                        char * grid_type;
                        if( ++i < argc )
                                grid_type = argv[i];
                        else
                                print_CLI_error();

                        if( strcmp(grid_type, "unionized") == 0 )
                                input.grid_type = UNIONIZED;
                        else if( strcmp(grid_type, "nuclide") == 0 )
                                input.grid_type = NUCLIDE;
                        else if( strcmp(grid_type, "hash") == 0 )
                                input.grid_type = HASH;
                        else
                                print_CLI_error();
                }
                // binary mode (-b)
                else if( strcmp(arg, "-b") == 0 )
                {
                        char * binary_mode;
                        if( ++i < argc )
                                binary_mode = argv[i];
                        else
                                print_CLI_error();

                        if( strcmp(binary_mode, "read") == 0 )
                                input.binary_mode = READ;
                        else if( strcmp(binary_mode, "write") == 0 )
                                input.binary_mode = WRITE;
                        else
                                print_CLI_error();
                }
                // kernel optimization selection (-k)
                else if( strcmp(arg, "-k") == 0 )
                {
                        if( ++i < argc )
                        {
                                input.kernel_id = atoi(argv[i]);
                        }
                        else
                                print_CLI_error();
                }
                // number of kernel iterations (-n)
                else if( strcmp(arg, "-n") == 0 )
                {
                        if( ++i < argc)
                        {
                                input.num_iterations = atoi(argv[i]);
                        }
                        else
                                print_CLI_error();
                }
                else if( strcmp(arg, "--csv") == 0 )
                {
                        if( ++i < argc ) {
                            input.filename = (char *)malloc(strlen(argv[i]) + 1);
                            strcpy(input.filename, argv[i]);
                        }
                        else
                                print_CLI_error();
                }
                // particles (-p)
                else if( strcmp(arg, "-w") == 0 )
                {
                        if( ++i < argc)
                        {
                                input.num_warmups = atoi(argv[i]);
                        }
                        else
                                print_CLI_error();
                }
                else
                        print_CLI_error();
        }

        // Validate Input

        // Validate nthreads
        if( input.nthreads < 1 )
                print_CLI_error();

        // Validate n_isotopes
        if( input.n_isotopes < 1 )
                print_CLI_error();

        // Validate n_gridpoints
        if( input.n_gridpoints < 1 )
                print_CLI_error();

        // Validate lookups
        if( input.lookups < 1 )
                print_CLI_error();

        // Validate Hash Bins
        if( input.hash_bins < 1 )
                print_CLI_error();

        // Validate number of iterations
        if ( input.num_iterations < 1 )
                print_CLI_error();

        // Validate HM size
        if( strcasecmp(input.HM, "small") != 0 &&
                strcasecmp(input.HM, "large") != 0 &&
                strcasecmp(input.HM, "XL") != 0 &&
                strcasecmp(input.HM, "XXL") != 0 )
                print_CLI_error();

        // Set HM size specific parameters
        // (defaults to large)
        if( strcasecmp(input.HM, "small") == 0 )
                input.n_isotopes = 68;
        else if( strcasecmp(input.HM, "XL") == 0 && user_g == 0 )
                input.n_gridpoints = 238847; // sized to make 120 GB XS data
        else if( strcasecmp(input.HM, "XXL") == 0 && user_g == 0 )
                input.n_gridpoints = 238847 * 2.1; // 252 GB XS data

        // Return input struct
        return input;
}

__host__ void binary_write( Inputs in, SimulationData SD )
{
        const char * fname = "XS_data.dat";
        printf("Writing all data structures to binary file %s...\n", fname);
        FILE * fp = fopen(fname, "w");

        // Write SimulationData Object. Include pointers, even though we won't be using them.
        fwrite(&SD, sizeof(SimulationData), 1, fp);

        // Write heap arrays in SimulationData Object
        fwrite(SD.num_nucs,       sizeof(int), SD.length_num_nucs, fp);
        fwrite(SD.concs,          sizeof(double), SD.length_concs, fp);
        fwrite(SD.mats,           sizeof(int), SD.length_mats, fp);
        fwrite(SD.nuclide_grid,   sizeof(NuclideGridPoint), SD.length_nuclide_grid, fp);
        fwrite(SD.index_grid, sizeof(int), SD.length_index_grid, fp);
        fwrite(SD.unionized_energy_array, sizeof(double), SD.length_unionized_energy_array, fp);

        fclose(fp);
}

__host__ SimulationData binary_read( Inputs in )
{
        SimulationData SD;

        const char * fname = "XS_data.dat";
        printf("Reading all data structures from binary file %s...\n", fname);

        FILE * fp = fopen(fname, "r");
        assert(fp != NULL);

        // Read SimulationData Object. Include pointers, even though we won't be using them.
        fread(&SD, sizeof(SimulationData), 1, fp);

        // Allocate space for arrays on heap
        SD.num_nucs = (int *) malloc(SD.length_num_nucs * sizeof(int));
        SD.concs = (double *) malloc(SD.length_concs * sizeof(double));
        SD.mats = (int *) malloc(SD.length_mats * sizeof(int));
        SD.nuclide_grid = (NuclideGridPoint *) malloc(SD.length_nuclide_grid * sizeof(NuclideGridPoint));
        SD.index_grid = (int *) malloc( SD.length_index_grid * sizeof(int));
        SD.unionized_energy_array = (double *) malloc( SD.length_unionized_energy_array * sizeof(double));

        // Read heap arrays into SimulationData Object
        fread(SD.num_nucs,       sizeof(int), SD.length_num_nucs, fp);
        fread(SD.concs,          sizeof(double), SD.length_concs, fp);
        fread(SD.mats,           sizeof(int), SD.length_mats, fp);
        fread(SD.nuclide_grid,   sizeof(NuclideGridPoint), SD.length_nuclide_grid, fp);
        fread(SD.index_grid, sizeof(int), SD.length_index_grid, fp);
        fread(SD.unionized_energy_array, sizeof(double), SD.length_unionized_energy_array, fp);

        fclose(fp);

        return SD;
}

#endif // XSBENCH_SHARED_HEADER_H