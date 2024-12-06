#include <stdio.h>
#include <stdlib.h>

// Shared utility function for parallel processing
void shared_function(int* array, int num_elements, double x) {
    for (int i = 0; i < num_elements; i++) {
        array[i] += x;
    }
}

void parallel_execute(int* data, int num_elements, char* filename) {
    // OpenMP directives to execute the kernel
#pragma omp parallel for reduction(sum)
    for (int i = 0; i < num_elements; i++) {
        shared_function(data, num_elements, data[i]);
    }

    // Check if output file should be generated
    FILE* output_file;
    if (filename) {
        output_file = fopen(filename, "w");
        if (!output_file) {
            printf("Failed to open output file %s\n", filename);
            exit(1);
        }
    } else {
        output_file = stdout;
    }

    // Print the result
    for (int i = 0; i < num_elements; i++) {
        fprintf(output_file, "%f ", data[i]);
    }
    fprintf(output_file, "\n");
    fflush(output_file);

    if (output_file != stdout) {
        fclose(output_file);
    }
}

// Main function to execute the program
int main() {
    int data[1000];
    parallel_execute(data, 1000, "result.txt");

    return 0;
}