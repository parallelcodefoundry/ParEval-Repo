#include <iostream>
#include <random>
#include <omp.h>

// Function to calculate cellsXOR
void cellsXOR(int* d_input, int* d_output, size_t N) {
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int count = 0;
            if (i > 0 && d_input[(i-1)*N + j] == 1) count++;
            if (i < N-1 && d_input[(i+1)*N + j] == 1) count++;
            if (j > 0 && d_input[i*N + (j-1)] == 1) count++;
            if (j < N-1 && d_input[i*N + (j+1)] == 1) count++;
            d_output[i*N + j] = (count == 1) ? 1 : 0;
        }
    }
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);

    // Initialize input and output arrays
    int N = 1024;
    int* d_input = new int[N*N];
    int* d_output = new int[N*N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            d_input[i*N + j] = dis(gen);
        }
    }

    // Set up OpenMP parameters
    #pragma omp parallel
    {
        int numBlocks = (N + 32 - 1) / 32;
        int blocksPerThreadGroup = 32;

        cellsXOR(d_input, d_output, N);

        for (int i = 0; i < numBlocks; i++) {
            #pragma omp for
            for (int j = 0; j < blocksPerThreadGroup; j++) {
                // Adjust thread indices for OpenMP parallelization
                int threadIdxX = i * blocksPerThreadGroup + j;
                int threadIdxY = j;

                if (threadIdxX < N / blocksPerThreadGroup && threadIdxY < N / blocksPerThreadGroup) {
                    d_output[threadIdxX*N + threadIdxY] = 0; // Initialize output array
                }
            }
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (d_input[i*N + j] == 1) d_output[i*N + j]++;
            }
        }
    }

    // Validate the output
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            if (d_input[(i-1)*N + j] == 1 && d_input[i*N + j] != 1) {
                std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
                break;
            }
        }
    }

    // Print the output
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (d_input[i*N + j] == 1) d_output[i*N + j]++;
        }
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            std::cout << d_output[i*N + j] << " ";
            if (i % N == N - 1) std::cout << std::endl;
        }
    }

    // Clean up
    delete[] d_input;
    delete[] d_output;

    return 0;
}