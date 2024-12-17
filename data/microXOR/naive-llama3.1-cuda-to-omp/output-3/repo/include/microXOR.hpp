#ifndef MICROXOR_H
#define MICROXOR_H

#include <iostream>
#include <random>
#include <omp.h>

#ifdef __OFFLOAD
  #define DEVICE omp_target_device_num(0)
#else
  #define DEVICE 0
#endif

__attribute__((offload(DEVICE, none))) void cellsXOR(int *input, int *output, size_t N);

#endif  // MICROXOR_H