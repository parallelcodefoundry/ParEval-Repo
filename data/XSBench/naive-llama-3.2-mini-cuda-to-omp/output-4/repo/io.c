#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

void reduce_add(int *result, int *x) {
    for (int i = 0; i < NUM_THREADS; i++) {
        result[i] += x[i];
    }
}

__global__ void io_cu_kernel(char *filename, char *output_filename, Inputs in, Profile *profile) {
    int grid_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (grid_id >= in.particles) return;

    if (in.binary_mode == 0) {
        // Unionized Grid
        int material_index = grid_id / NUM_THREADS;
        int energy_index = grid_id % NUM_THREADS;
        char *material_data = in.HM + material_index * sizeof(char);
        char *energy_data = in.HM + material_index * sizeof(char) + energy_index;

        // Simulate the binary search
        int low = 0;
        int high = 10000; // Assuming maximum possible value for energy index
        while (low <= high) {
            int mid = (low + high) / 2;
            char material_key = material_data[mid];
            char energy_key = energy_data[mid];

            if ((material_key == in.HM[grid_id * NUM_THREADS]) && (energy_key == in.HM[(grid_id * NUM_THREADS) + 1])) {
                // Match found, perform calculations
                int temp_result;
                reduce_add(&temp_result, &in.lookups);

                profile->kernel_time += 1; // Increment kernel time

                // Simulate calculation (no actual calculation performed)
                break;

            } else if ((material_key < in.HM[grid_id * NUM_THREADS]) && (energy_key >= in.HM[(grid_id * NUM_threads) + 1])) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

    } else {
        // Nuclide Grid
        int material_index = grid_id / NUM_THREADS;
        char *material_data = in.HM + material_index * sizeof(char);

        // Simulate the binary search
        int low = 0;
        int high = 10000; // Assuming maximum possible value for energy index
        while (low <= high) {
            int mid = (low + high) / 2;

            char material_key = material_data[mid];
            if (material_key == in.HM[grid_id * NUM_THREADS]) {
                // Match found, perform calculations
                int temp_result;
                reduce_add(&temp_result, &in.lookups);

                profile->kernel_time += 1; // Increment kernel time

                // Simulate calculation (no actual calculation performed)
                break;

            } else if (material_key < in.HM[grid_id * NUM_THREADS]) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

    }

}

int main() {
    Inputs input;
    // Initialize input struct
    Profile profile;
    // Initialize profile struct

    io_cu_kernel<<<NUM_THREADS, NUM_THREADS>>>(input.HM, output_filename, input, &profile);

    return 0;
}