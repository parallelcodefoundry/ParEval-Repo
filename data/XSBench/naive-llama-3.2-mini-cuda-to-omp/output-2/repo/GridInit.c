#include <iostream>
#include <omp.h>

// Define the number of threads for OpenMP
#define NUM_THREADS 4

// Function to initialize the grid and perform a lookup on each particle
void init_grid(Particle particles[], int num_particles, char *filename) {
    // Initialize the grid dimensions
    int ngrid = 100;
    int ngamma = 10;

    // Perform OpenMP parallelization for the first iteration of the binary search
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < ngrid; ++i) {
        for (int j = 0; j < ngamma; ++j) {
            double energy_value = particles[i][j].energy;
            int index = std::find(particles, particles + ngrid * ngamma, Particle(energy_value)) - particles;
            if (index >= 0) {
                // Perform a lookup on the particle
                // Replace this with your actual lookup function
                std::cout << "Lookup performed on particle at energy " << energy_value << std::endl;
            }
        }
    }

    // Perform OpenMP parallelization for the second iteration of the binary search
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < ngrid; ++i) {
        for (int j = 0; j < ngamma; ++j) {
            double energy_value = particles[i][j].energy;
            int index = std::find(particles, particles + ngrid * ngamma, Particle(energy_value)) - particles;
            if (index >= 0) {
                // Perform a lookup on the particle
                // Replace this with your actual lookup function
                std::cout << "Lookup performed on particle at energy " << energy_value << std::endl;
            }
        }
    }

    // Save the results to a file
    FILE* output = fopen(filename, "w");
    for (int i = 0; i < ngrid; ++i) {
        for (int j = 0; j < ngamma; ++j) {
            double energy_value = particles[i][j].energy;
            int index = std::find(particles, particles + ngrid * ngamma, Particle(energy_value)) - particles;
            if (index >= 0) {
                // Save the results to the file
                fprintf(output, "%f\n", energy_value);
            }
        }
    }
    fclose(output);
}

// Structure representing a particle with energy values
struct Particle {
    double energy[10];
};

int main() {
    Particle particles[100][10];

    init_grid(particles, 100 * 10, "output.txt");

    return 0;
}