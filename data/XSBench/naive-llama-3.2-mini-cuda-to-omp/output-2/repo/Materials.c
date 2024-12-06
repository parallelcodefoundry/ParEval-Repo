__global__ void materials_kernel(int* HM, Inputs* inputs) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < inputs->n_isotopes) {
    // Determine material based on current isotopes lookup iteration
    int mat = get_material(HM[idx], inputs);

    // Perform calculations for this material
    if (mat == 0) { // Deuterium
      deuterium(HM[idx], inputs);
    } else if (mat == 1) { // Helium-3
      helium_3(HM[idx], inputs);
    } else if (mat == 2) { // Oxygen-17
      oxygen_17(HM[idx], inputs);
    } // ... add more materials as needed

    __syncthreads();

    if (inputs->binary_mode) {
      binary_search(HM[idx], inputs);
    }
  }
}

__global__ void materials_kernel_openmp(int* HM, Inputs* inputs) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < inputs->n_isotopes) {
    // Determine material based on current isotopes lookup iteration
    int mat = get_material(HM[idx], inputs);

    // Perform calculations for this material
    if (mat == 0) { // Deuterium
      deuterium_openmp(HM[idx], inputs);
    } else if (mat == 1) { // Helium-3
      helium_3_openmp(HM[idx], inputs);
    } else if (mat == 2) { // Oxygen-17
      oxygen_17_openmp(HM[idx], inputs);
    } // ... add more materials as needed

    __syncthreads();

    #pragma omp task
    {
      binary_search_openmp(HM[idx], inputs);
    }
  }
}

void run_materials_kernel(int* HM, Inputs* inputs) {
  int num_threads = get_num_threads();
  int num_blocks = (inputs->n_isotopes + num_threads - 1) / num_threads;
  launch_kernel(materials_kernel, num_blocks, num_threads, HM, inputs);
}

void run_materials_kernel_openmp(int* HM, Inputs* inputs) {
  int num_threads = get_num_threads();
  int num_blocks = (inputs->n_isotopes + num_threads - 1) / num_threads;
  launch_kernel(materials_kernel_openmp, num_blocks, num_threads, HM, inputs);
}

int get_material(int HM_idx, Inputs* inputs) {
  // implementation of material determination
  return 0; // default to deuterium for now
}

void deuterium(int HM_idx, Inputs* inputs) {
  // implementation of deuterium calculations
}

void helium_3(int HM_idx, Inputs* inputs) {
  // implementation of helium-3 calculations
}

void oxygen_17(int HM_idx, Inputs* inputs) {
  // implementation of oxygen-17 calculations
}

void binary_search(int HM_idx, Inputs* inputs) {
  // implementation of binary search for nuclide lookups
}

void deuterium_openmp(int HM_idx, Inputs* inputs) {
  // implementation of deuterium OpenMP kernel
}

void helium_3_openmp(int HM_idx, Inputs* inputs) {
  // implementation of helium-3 OpenMP kernel
}

void oxygen_17_openmp(int HM_idx, Inputs* inputs) {
  // implementation of oxygen-17 OpenMP kernel
}

// ... add more materials and their implementations as needed

int main() {
  int HM_size = inputs->n_isotopes;
  int num_threads = get_num_threads();
  int num_blocks = (HM_size + num_threads - 1) / num_threads;

  // Run openMP version of the kernel
  run_materials_kernel_openmp(HM, inputs);

  return 0;
}