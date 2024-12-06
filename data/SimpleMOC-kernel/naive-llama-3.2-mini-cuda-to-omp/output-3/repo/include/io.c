#include <omp.h>

// Function to perform IO operations (e.g., read and write data)
void ioOperations(float *data, int numElements) {
    // Open a file for reading and writing
#ifdef _OPENMP
#pragma omp target teams distribute parallel for
#endif
    FILE *fp;
    if ((fp = fopen("input.txt", "r")) == NULL) {
        printf("Error opening input file.\n");
        exit(1);
    }

    // Read data from the file
    for (int i = 0; i < numElements; i++) {
        fscanf(fp, "%f", &data[i]);
    }

    fclose(fp);

    if ((fp = fopen("output.txt", "w")) == NULL) {
        printf("Error opening output file.\n");
        exit(1);
    }

    // Write data to the file
#ifdef _OPENMP
#pragma omp target teams distribute parallel for
#endif
    for (int i = 0; i < numElements; i++) {
        fprintf(fp, "%f", data[i]);
    }

    fclose(fp);
}