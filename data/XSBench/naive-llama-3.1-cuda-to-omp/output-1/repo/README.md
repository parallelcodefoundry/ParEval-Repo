// XSbench_shared_header.h:
#ifndef XSBENCH_SHARED_HEADER_H
#define XSBENCH_SHARED_HEADER_H

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

inline void print_profile(Profile profile, Inputs in) {
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

#endif // XSBENCH_SHARED_HEADER_H

// Inputs Struct for OpenMP-Offload Execution Model
typedef struct {
    int num_teams;
    int num_threads_per_team;
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
} Inputs_OpenMP;

// Profile Struct for OpenMP-Offload Execution Model
typedef struct {
    double device_to_host_time;
    double kernel_time;
    double host_to_device_time;
} Profile_OpenMP;

// Function to Print Profile in OpenMP-Offload Execution Model
inline void print_profile_OpenMP(Profile_OpenMP profile, Inputs_OpenMP in) {
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

// Function to Initialize OpenMP-Offload Execution Model
void init_OpenMP() {
    int nthreads = omp_get_max_threads();
    printf("Number of threads: %d\n", nthreads);
}

// Main Function for OpenMP-Offload Execution Model
int main() {
    Inputs_OpenMP in;
    Profile_OpenMP profile;

    // Initialize OpenMP-Offload Execution Model
    init_OpenMP();

    // Set up input parameters
    in.num_teams = 4;  // Number of teams
    in.num_threads_per_team = 8;  // Number of threads per team

    // Run simulation
    run_simulation(in);

    // Get profile data
    profile.device_to_host_time = get_device_to_host_time();
    profile.kernel_time = get_kernel_time();
    profile.host_to_device_time = get_host_to_device_time();

    // Print profile data
    print_profile_OpenMP(profile, in);

    return 0;
}

// Function to Run Simulation in OpenMP-Offload Execution Model
void run_simulation(Inputs_OpenMP in) {
    #pragma omp parallel num_threads(in.num_threads_per_team)
    {
        int tid = omp_get_thread_num();

        // Set up local input parameters
        Inputs loc_in;

        // Initialize local variables
        char * HM;
        int grid_type;
        int hash_bins;
        int particles;
        int simulation_method;
        int binary_mode;
        int kernel_id;
        int num_iterations;
        int num_warmups;

        // Set up local input parameters from global input parameters
        loc_in.HM = in.HM + tid * sizeof(char);
        loc_in.grid_type = in.grid_type;
        loc_in.hash_bins = in.hash_bins;
        loc_in.particles = in.particles;
        loc_in.simulation_method = in.simulation_method;
        loc_in.binary_mode = in.binary_mode;
        loc_in.kernel_id = in.kernel_id;
        loc_in.num_iterations = in.num_iterations;
        loc_in.num_warmups = in.num_warmups;

        // Run kernel
        run_kernel(loc_in);

    }
}

// Function to Get Device-to-Host Time in OpenMP-Offload Execution Model
double get_device_to_host_time() {
    return 0.12345;  // Example value
}

// Function to Get Kernel Time in OpenMP-Offload Execution Model
double get_kernel_time() {
    return 0.67890;  // Example value
}

// Function to Get Host-to-Device Time in OpenMP-Offload Execution Model
double get_host_to_device_time() {
    return 0.23456;  // Example value
}