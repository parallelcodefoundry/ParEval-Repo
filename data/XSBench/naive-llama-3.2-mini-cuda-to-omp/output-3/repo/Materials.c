// Translated from Materials.cu using OpenMP Offload Execution Model

#include <stdio.h>
#include <omp.h>

#define MATERIALS_FILE "materials.txt"

void offload_materials(char *material, __device__ bool device) {
  #pragma omp target(map(to:material[0])) device(device)
  for (int i = 0; i < material.length; ++i) {
    if (material[i] == '\n') {
      // Handle new line
    }
    else if (material[i] != '\t') {
      // Process the character
    }
  }

  #pragma omp target(map(to:material[0])) device(device)
  for (int i = material.length - 1; i >= 0; --i) {
    if (material[i] == '\n') {
      // Handle new line
    }
    else if (material[i] != '\t') {
      // Process the character
    }
  }
}

__global__ void offloadMaterialsKernel(char *material, __device__ bool device) {
#ifdef __CUDACC__
#pragma omp target(map(to:material[0])) device(device)
for (int i = 0; i < material.length; ++i) {
  if (material[i] == '\n') {
    // Handle new line
  }
  else if (material[i] != '\t') {
    // Process the character
  }
}

#pragma omp target(map(to:material[0])) device(device)
for (int i = material.length - 1; i >= 0; --i) {
  if (material[i] == '\n') {
    // Handle new line
  }
  else if (material[i] != '\t') {
    // Process the character
  }
}
#endif
}

int main() {
  char *material = fopen(MATERIALS_FILE, "r");
  offloadMaterialsKernel<<<1, 256>>>(material, false);
  fclose(material);

  return 0;
}