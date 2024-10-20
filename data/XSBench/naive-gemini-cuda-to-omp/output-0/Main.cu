```c++
#include "XSbench_header.cuh"

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
    border_print();
    center_print("SIMULATION", 79);
    border_print();
  }

  // Start Simulation Timer
  omp_start = get_time();

  // Run simulation
  if (in.simulation_method == EVENT_BASED) {
    if (in.kernel_id == 0) {
#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
      for (int i = 0; i < in.lookups; i++) {
        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2 * i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat = pick_mat(&seed);

        double macro_xs_vector[5] = {0};

        // Perform macroscopic Cross Section Lookup
        calculate_macro_xs(
            p_energy,        // Sampled neutron energy (in lethargy)
            mat,             // Sampled material type index neutron is in
            in.n_isotopes,   // Total number of isotopes in simulation
            in.n_gridpoints, // Number of gridpoints per isotope in simulation
            SD.num_nucs,     // 1-D array with number of nuclides per material
            SD.concs,        // Flattened 2-D array with concentration of each
                            // nuclide in each material
            SD.unionized_energy_array, // 1-D Unionized energy array
            SD.index_grid,   // Flattened 2-D grid holding indices into nuclide
                            // grid for each unionized energy level
            SD.nuclide_grid, // Flattened 2-D grid holding energy levels and
                            // XS_data for all nuclides in simulation
            SD.mats,         // Flattened 2-D array with nuclide indices
                            // defining composition of each type of material
            macro_xs_vector, // 1-D array with result of the macroscopic cross
                            // section (5 different reaction channels)
            in.grid_type,    // Lookup type (nuclide, hash, or unionized)
            in.hash_bins,    // Number of hash bins used (if using hash lookup
                            // type)
            SD.max_num_nucs  // Maximum number of nuclides present in any
                            // material
        );

        // For verification, and to prevent the compiler from optimizing
        // all work out, we interrogate the returned macro_xs_vector array
        // to find its maximum value index, then increment the verification
        // value by that index. In this implementation, we have each thread
        // write to its thread_id index in an array, which we will reduce
        // with a thrust reduction kernel after the main simulation kernel.
        double max = -1.0;
        int max_idx = 0;
        for (int j = 0; j < 5; j++) {
          if (macro_xs_vector[j] > max) {
            max = macro_xs_vector[j];
            max_idx = j;
          }
        }
        SD.verification[i] = max_idx + 1;
      }
    } else if (in.kernel_id == 1) {
      // Allocate Additional Data Structures Needed by Optimized Kernel
      if (mype == 0)
        printf("Allocating additional device data required by kernel...\n");
      size_t sz;
      size_t total_sz = 0;

      sz = in.lookups * sizeof(double);
      SD.p_energy_samples = (double *)malloc(sz);
      total_sz += sz;
      SD.length_p_energy_samples = in.lookups;

      sz = in.lookups * sizeof(int);
      SD.mat_samples = (int *)malloc(sz);
      total_sz += sz;
      SD.length_mat_samples = in.lookups;

      if (mype == 0)
        printf(
            "Allocated an additional %.0lf MB of data on GPU.\n",
            total_sz / 1024.0 / 1024.0);

      // Configure & Launch Simulation Kernel
      if (mype == 0)
        printf("Beginning optimized simulation...\n");

      int nthreads = 32;
      int nblocks = ceil((double)in.lookups / 32.0);

#pragma omp target teams distribute parallel for map(tofrom:SD)
      for (int i = 0; i < in.lookups; i++) {
        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2 * i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat = pick_mat(&seed);

        // Store sample data in state array
        SD.p_energy_samples[i] = p_energy;
        SD.mat_samples[i] = mat;
      }

#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
      for (int i = 0; i < in.lookups; i++) {
        double macro_xs_vector[5] = {0};

        // Perform macroscopic Cross Section Lookup
        calculate_macro_xs(
            SD.p_energy_samples[i],        // Sampled neutron energy (in
                                            // lethargy)
            SD.mat_samples[i],             // Sampled material type index
                                            // neutron is in
            in.n_isotopes,   // Total number of isotopes in simulation
            in.n_gridpoints, // Number of gridpoints per isotope in
                            // simulation
            SD.num_nucs,     // 1-D array with number of nuclides per material
            SD.concs,        // Flattened 2-D array with concentration of
                            // each nuclide in each material
            SD.unionized_energy_array, // 1-D Unionized energy array
            SD.index_grid,   // Flattened 2-D grid holding indices into
                            // nuclide grid for each unionized energy level
            SD.nuclide_grid, // Flattened 2-D grid holding energy levels and
                            // XS_data for all nuclides in simulation
            SD.mats,         // Flattened 2-D array with nuclide indices
                            // defining composition of each type of material
            macro_xs_vector, // 1-D array with result of the macroscopic cross
                            // section (5 different reaction channels)
            in.grid_type,    // Lookup type (nuclide, hash, or unionized)
            in.hash_bins,    // Number of hash bins used (if using hash
                            // lookup type)
            SD.max_num_nucs  // Maximum number of nuclides present in any
                            // material
        );

        // For verification, and to prevent the compiler from optimizing
        // all work out, we interrogate the returned macro_xs_vector array
        // to find its maximum value index, then increment the verification
        // value by that index. In this implementation, we have each thread
        // write to its thread_id index in an array, which we will reduce
        // with a thrust reduction kernel after the main simulation kernel.
        double max = -1.0;
        int max_idx = 0;
        for (int j = 0; j < 5; j++) {
          if (macro_xs_vector[j] > max) {
            max = macro_xs_vector[j];
            max_idx = j;
          }
        }
        SD.verification[i] = max_idx + 1;
      }
    } else if (in.kernel_id == 2) {
      // Allocate Additional Data Structures Needed by Optimized Kernel
      if (mype == 0)
        printf("Allocating additional device data required by kernel...\n");
      size_t sz;
      size_t total_sz = 0;

      sz = in.lookups * sizeof(double);
      SD.p_energy_samples = (double *)malloc(sz);
      total_sz += sz;
      SD.length_p_energy_samples = in.lookups;

      sz = in.lookups * sizeof(int);
      SD.mat_samples = (int *)malloc(sz);
      total_sz += sz;
      SD.length_mat_samples = in.lookups;

      if (mype == 0)
        printf(
            "Allocated an additional %.0lf MB of data on GPU.\n",
            total_sz / 1024.0 / 1024.0);

      // Configure & Launch Simulation Kernel
      if (mype == 0)
        printf("Beginning optimized simulation...\n");

      int nthreads = 32;
      int nblocks = ceil((double)in.lookups / 32.0);

#pragma omp target teams distribute parallel for map(tofrom:SD)
      for (int i = 0; i < in.lookups; i++) {
        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2 * i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat = pick_mat(&seed);

        // Store sample data in state array
        SD.p_energy_samples[i] = p_energy;
        SD.mat_samples[i] = mat;
      }

      for (int m = 0; m < 12; m++) {
#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
        for (int i = 0; i < in.lookups; i++) {
          // Check that our material type matches the kernel material
          int mat = SD.mat_samples[i];
          if (mat != m)
            continue;

          double macro_xs_vector[5] = {0};

          // Perform macroscopic Cross Section Lookup
          calculate_macro_xs(
              SD.p_energy_samples[i],        // Sampled neutron energy (in
                                              // lethargy)
              mat,             // Sampled material type index neutron is
                              // in
              in.n_isotopes,   // Total number of isotopes in simulation
              in.n_gridpoints, // Number of gridpoints per isotope in
                              // simulation
              SD.num_nucs,     // 1-D array with number of nuclides per
                              // material
              SD.concs,        // Flattened 2-D array with concentration of
                              // each nuclide in each material
              SD.unionized_energy_array, // 1-D Unionized energy array
              SD.index_grid,   // Flattened 2-D grid holding indices into
                              // nuclide grid for each unionized energy
                              // level
              SD.nuclide_grid, // Flattened 2-D grid holding energy levels
                              // and XS_data for all nuclides in simulation
              SD.mats,         // Flattened 2-D array with nuclide indices
                              // defining composition of each type of material
              macro_xs_vector, // 1-D array with result of the macroscopic
                              // cross section (5 different reaction channels)
              in.grid_type,    // Lookup type (nuclide, hash, or unionized)
              in.hash_bins,    // Number of hash bins used (if using hash
                              // lookup type)
              SD.max_num_nucs  // Maximum number of nuclides present in any
                              // material
          );

          // For verification, and to prevent the compiler from optimizing
          // all work out, we interrogate the returned macro_xs_vector array
          // to find its maximum value index, then increment the verification
          // value by that index. In this implementation, we have each thread
          // write to its thread_id index in an array, which we will reduce
          // with a thrust reduction kernel after the main simulation kernel.
          double max = -1.0;
          int max_idx = 0;
          for (int j = 0; j < 5; j++) {
            if (macro_xs_vector[j] > max) {
              max = macro_xs_vector[j];
              max_idx = j;
            }
          }
          SD.verification[i] = max_idx + 1;
        }
      }
    } else if (in.kernel_id == 3) {
      // Allocate Additional Data Structures Needed by Optimized Kernel
      if (mype == 0)
        printf("Allocating additional device data required by kernel...\n");
      size_t sz;
      size_t total_sz = 0;

      sz = in.lookups * sizeof(double);
      SD.p_energy_samples = (double *)malloc(sz);
      total_sz += sz;
      SD.length_p_energy_samples = in.lookups;

      sz = in.lookups * sizeof(int);
      SD.mat_samples = (int *)malloc(sz);
      total_sz += sz;
      SD.length_mat_samples = in.lookups;

      if (mype == 0)
        printf(
            "Allocated an additional %.0lf MB of data on GPU.\n",
            total_sz / 1024.0 / 1024.0);

      // Configure & Launch Simulation Kernel
      if (mype == 0)
        printf("Beginning optimized simulation...\n");

      int nthreads = 32;
      int nblocks = ceil((double)in.lookups / 32.0);

#pragma omp target teams distribute parallel for map(tofrom:SD)
      for (int i = 0; i < in.lookups; i++) {
        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2 * i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat = pick_mat(&seed);

        // Store sample data in state array
        SD.p_energy_samples[i] = p_energy;
        SD.mat_samples[i] = mat;
      }

      for (int is_fuel = 0; is_fuel < 2; is_fuel++) {
#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
        for (int i = 0; i < in.lookups; i++) {
          int mat = SD.mat_samples[i];

          // If this is the fuel kernel, AND this is a fuel lookup, then
          // perform a lookup
          // OR if this is not the fuel kernel, AND this is not a fuel
          // lookup, then perform the lookup
          if (((is_fuel == 1) && (mat == 0)) ||
              ((is_fuel == 0) && (mat != 0))) {
            double macro_xs_vector[5] = {0};

            // Perform macroscopic Cross Section Lookup
            calculate_macro_xs(
                SD.p_energy_samples[i],        // Sampled neutron energy (in
                                                // lethargy)
                mat,             // Sampled material type index neutron is
                                // in
                in.n_isotopes,   // Total number of isotopes in simulation
                in.n_gridpoints, // Number of gridpoints per isotope in
                                // simulation
                SD.num_nucs,     // 1-D array with number of nuclides per
                                // material
                SD.concs,        // Flattened 2-D array with concentration of
                                // each nuclide in each material
                SD.unionized_energy_array, // 1-D Unionized energy array
                SD.index_grid,   // Flattened 2-D grid holding indices into
                                // nuclide grid for each unionized energy
                                // level
                SD.nuclide_grid, // Flattened 2-D grid holding energy levels
                                // and XS_data for all nuclides in simulation
                SD.mats,         // Flattened 2-D array with nuclide indices
                                // defining composition of each type of
                                // material
                macro_xs_vector, // 1-D array with result of the macroscopic
                                // cross section (5 different reaction
                                // channels)
                in.grid_type,    // Lookup type (nuclide, hash, or unionized)
                in.hash_bins,    // Number of hash bins used (if using hash
                                // lookup type)
                SD.max_num_nucs  // Maximum number of nuclides present in any
                                // material
            );

            // For verification, and to prevent the compiler from optimizing
            // all work out, we interrogate the returned macro_xs_vector array
            // to find its maximum value index, then increment the verification
            // value by that index. In this implementation, we have each thread
            // write to its thread_id index in an array, which we will reduce
            // with a thrust reduction kernel after the main simulation kernel.
            double max = -1.0;
            int max_idx = 0;
            for (int j = 0; j < 5; j++) {
              if (macro_xs_vector[j] > max) {
                max = macro_xs_vector[j];
                max_idx = j;
              }
            }
            SD.verification[i] = max_idx + 1;
          }
        }
      }
    } else if (in.kernel_id == 4) {
      // Allocate Additional Data Structures Needed by Optimized Kernel
      if (mype == 0)
        printf("Allocating additional device data required by kernel...\n");
      size_t sz;
      size_t total_sz = 0;

      sz = in.lookups * sizeof(double);
      SD.p_energy_samples = (double *)malloc(sz);
      total_sz += sz;
      SD.length_p_energy_samples = in.lookups;

      sz = in.lookups * sizeof(int);
      SD.mat_samples = (int *)malloc(sz);
      total_sz += sz;
      SD.length_mat_samples = in.lookups;

      if (mype == 0)
        printf(
            "Allocated an additional %.0lf MB of data on GPU.\n",
            total_sz / 1024.0 / 1024.0);

      // Configure & Launch Simulation Kernel
      if (mype == 0)
        printf("Beginning optimized simulation...\n");

      int nthreads = 32;
      int nblocks = ceil((double)in.lookups / 32.0);

#pragma omp target teams distribute parallel for map(tofrom:SD)
      for (int i = 0; i < in.lookups; i++) {
        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2 * i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat = pick_mat(&seed);

        // Store sample data in state array
        SD.p_energy_samples[i] = p_energy;
        SD.mat_samples[i] = mat;
      }

      // Count the number of fuel material lookups that need to be performed
      // (fuel id = 0)
      int n_lookups_per_material[12];
      for (int m = 0; m < 12; m++) {
        n_lookups_per_material[m] = 0;
        for (int i = 0; i < in.lookups; i++) {
          if (SD.mat_samples[i] == m)
            n_lookups_per_material[m]++;
        }
      }

      // Sort materials
      for (int i = 0; i < in.lookups - 1; i++) {
        for (int j = i + 1; j < in.lookups; j++) {
          if (SD.mat_samples[i] > SD.mat_samples[j]) {
            int temp = SD.mat_samples[i];
            SD.mat_samples[i] = SD.mat_samples[j];
            SD.mat_samples[j] = temp;
            double temp2 = SD.p_energy_samples[i];
            SD.p_energy_samples[i] = SD.p_energy_samples[j];
            SD.p_energy_samples[j] = temp2;
          }
        }
      }

      // Launch all material kernels individually
      int offset = 0;
      for (int m = 0; m < 12; m++) {
        nthreads = 32;
        nblocks = ceil((double)n_lookups_per_material[m] /
                       (double)nthreads);
#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
        for (int i = 0; i < n_lookups_per_material[m]; i++) {
          int j = i + offset;

          // Check that our material type matches the kernel material
          int mat = SD.mat_samples[j];
          if (mat != m)
            continue;

          double macro_xs_vector[5] = {0};

          // Perform macroscopic Cross Section Lookup
          calculate_macro_xs(
              SD.p_energy_samples[j],        // Sampled neutron energy (in
                                              // lethargy)
              mat,             // Sampled material type index neutron is
                              // in
              in.n_isotopes,   // Total number of isotopes in simulation
              in.n_gridpoints, // Number of gridpoints per isotope in
                              // simulation
              SD.num_nucs,     // 1-D array with number of nuclides per
                              // material
              SD.concs,        // Flattened 2-D array with concentration of
                              // each nuclide in each material
              SD.unionized_energy_array, // 1-D Unionized energy array
              SD.index_grid,   // Flattened 2-D grid holding indices into
                              // nuclide grid for each unionized energy
                              // level
              SD.nuclide_grid, // Flattened 2-D grid holding energy levels
                              // and XS_data for all nuclides in simulation
              SD.mats,         // Flattened 2-D array with nuclide indices
                              // defining composition of each type of material
              macro_xs_vector, // 1-D array with result of the macroscopic
                              // cross section (5 different reaction channels)
              in.grid_type,    // Lookup type (nuclide, hash, or unionized)
              in.hash_bins,    // Number of hash bins used (if using hash
                              // lookup type)
              SD.max_num_nucs  // Maximum number of nuclides present in any
                              // material
          );

          // For verification, and to prevent the compiler from optimizing
          // all work out, we interrogate the returned macro_xs_vector array
          // to find its maximum value index, then increment the verification
          // value by that index. In this implementation, we have each thread
          // write to its thread_id index in an array, which we will reduce
          // with a thrust reduction kernel after the main simulation kernel.
          double max = -1.0;
          int max_idx = 0;
          for (int k = 0; k < 5; k++) {
            if (macro_xs_vector[k] > max) {
              max = macro_xs_vector[k];
              max_idx = k;
            }
          }
          SD.verification[j] = max_idx + 1;
        }
        offset += n_lookups_per_material[m];
      }
    } else if (in.kernel_id == 5) {
      // Allocate Additional Data Structures Needed by Optimized Kernel
      if (mype == 0)
        printf("Allocating additional device data required by kernel...\n");
      size_t sz;
      size_t total_sz = 0;

      sz = in.lookups * sizeof(double);
      SD.p_energy_samples = (double *)malloc(sz);
      total_sz += sz;
      SD.length_p_energy_samples = in.lookups;

      sz = in.lookups * sizeof(int);
      SD.mat_samples = (int *)malloc(sz);
      total_sz += sz;
      SD.length_mat_samples = in.lookups;

      if (mype == 0)
        printf(
            "Allocated an additional %.0lf MB of data on GPU.\n",
            total_sz / 1024.0 / 1024.0);

      // Configure & Launch Simulation Kernel
      if (mype == 0)
        printf("Beginning optimized simulation...\n");

      int nthreads = 32;
      int nblocks = ceil((double)in.lookups / 32.0);

#pragma omp target teams distribute parallel for map(tofrom:SD)
      for (int i = 0; i < in.lookups; i++) {
        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2 * i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat = pick_mat(&seed);

        // Store sample data in state array
        SD.p_energy_samples[i] = p_energy;
        SD.mat_samples[i] = mat;
      }

      // Count the number of fuel material lookups that need to be performed
      // (fuel id = 0)
      int n_fuel_lookups = 0;
      for (int i = 0; i < in.lookups; i++) {
        if (SD.mat_samples[i] == 0)
          n_fuel_lookups++;
      }

      // Partition fuel into the first part of the array
      for (int i = 0; i < in.lookups - 1; i++) {
        for (int j = i + 1; j < in.lookups; j++) {
          if ((SD.mat_samples[i] == 0) && (SD.mat_samples[j] != 0)) {
            int temp = SD.mat_samples[i];
            SD.mat_samples[i] = SD.mat_samples[j];
            SD.mat_samples[j] = temp;
            double temp2 = SD.p_energy_samples[i];
            SD.p_energy_samples[i] = SD.p_energy_samples[j];
            SD.p_energy_samples[j] = temp2;
          }
        }
      }

      // Launch all material kernels individually (asynchronous is allowed)
      nblocks = ceil((double)n_fuel_lookups / (double)nthreads);
#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
      for (int i = 0; i < n_fuel_lookups; i++) {
        double macro_xs_vector[5] = {0};

        // Perform macroscopic Cross Section Lookup
        calculate_macro_xs(
            SD.p_energy_samples[i],        // Sampled neutron energy (in
                                            // lethargy)
            SD.mat_samples[i],             // Sampled material type index
                                            // neutron is in
            in.n_isotopes,   // Total number of isotopes in simulation
            in.n_gridpoints, // Number of gridpoints per isotope in
                            // simulation
            SD.num_nucs,     // 1-D array with number of nuclides per material
            SD.concs,        // Flattened 2-D array with concentration of
                            // each nuclide in each material
            SD.unionized_energy_array, // 1-D Unionized energy array
            SD.index_grid,   // Flattened 2-D grid holding indices into
                            // nuclide grid for each unionized energy level
            SD.nuclide_grid, // Flattened 2-D grid holding energy levels and
                            // XS_data for all nuclides in simulation
            SD.mats,         // Flattened 2-D array with nuclide indices
                            // defining composition of each type of material
            macro_xs_vector, // 1-D array with result of the macroscopic cross
                            // section (5 different reaction channels)
            in.grid_type,    // Lookup type (nuclide, hash, or unionized)
            in.hash_bins,    // Number of hash bins used (if using hash
                            // lookup type)
            SD.max_num_nucs  // Maximum number of nuclides present in any
                            // material
        );

        // For verification, and to prevent the compiler from optimizing
        // all work out, we interrogate the returned macro_xs_vector array
        // to find its maximum value index, then increment the verification
        // value by that index. In this implementation, we have each thread
        // write to its thread_id index in an array, which we will reduce
        // with a thrust reduction kernel after the main simulation kernel.
        double max = -1.0;
        int max_idx = 0;
        for (int j = 0; j < 5; j++) {
          if (macro_xs_vector[j] > max) {
            max = macro_xs_vector[j];
            max_idx = j;
          }
        }
        SD.verification[i] = max_idx + 1;
      }

      nblocks = ceil((double)(in.lookups - n_fuel_lookups) /
                     (double)nthreads);
#pragma omp target teams distribute parallel for map(tofrom:SD) \
    reduction(+:verification)
      for (int i = 0; i < in.lookups - n_fuel_lookups; i++) {
        int j = i + n_fuel_lookups;

        double macro_xs_vector[5] = {0};

        // Perform macroscopic Cross Section Lookup
        calculate_macro_xs(
            SD.p_energy_samples[j],        // Sampled neutron energy (in
                                            // lethargy)
            SD.mat_samples[j],             // Sampled material type index
                                            // neutron is in
            in.n_isotopes,   // Total number of isotopes in simulation
            in.n_gridpoints, // Number of gridpoints per isotope in
                            // simulation
            SD.num_nucs,     // 1-D array with number of nuclides per material
            SD.concs,        // Flattened 2-D array with concentration of
                            // each nuclide in each material
            SD.unionized_energy_array, // 1-D Unionized energy array
            SD.index_grid,   // Flattened 2-D grid holding indices into
                            // nuclide grid for each unionized energy level
            SD.nuclide_grid, // Flattened 2-D grid holding energy levels and
                            // XS_data for all nuclides in simulation
            SD.mats,         // Flattened 2-D array with nuclide indices
                            // defining composition of each type of material
            macro_xs_vector, // 1-D array with result of the macroscopic cross
                            // section (5 different reaction channels)
            in.grid_type,    // Lookup type (nuclide, hash, or unionized)
            in.hash_bins,    // Number of hash bins used (if using hash
                            // lookup type)
            SD.max_num_nucs  // Maximum number of nuclides present in any
                            // material
        );

        // For verification, and to prevent the compiler from optimizing
        // all work out, we interrogate the returned macro_xs_vector array
        // to find its maximum value index, then increment the verification
        // value by that index. In this implementation, we have each thread
        // write to its thread_id index in an array, which we will reduce
        // with a thrust reduction kernel after the main simulation kernel.
        double max = -1.0;
        int max_idx = 0;
        