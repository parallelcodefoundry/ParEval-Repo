#ifndef SIMPLE_MOC_KERNEL_HEADER_H
#define SIMPLE_MOC_KERNEL_HEADER_H

#include <omp.h>

// Define a macro to check CUDA errors in offloaded regions
#define __cudaCheckErrorCudaOffload(file, line) \
  do { \
    int __err = omp_get_overall_verbosity(); \
    if (__err > 0 && !omp_in_parallel()) { \
      fprintf(stderr, "CUDA error at %s:%i\n", file, line); \
      exit(1); \
    } else if (omp_is_initial_device() && __err == 2) { \
      omp_set_verbosity(0); \
      cudaError_t err = cudaGetLastError(); \
      if (cudaSuccess != err) { \
        fprintf(stderr, "CUDA error at %s:%i : %s\n", file, line, cudaGetErrorString(err)); \
        exit(1); \
      } \
    } else if (__err == 3) { \
      int __cudaDev = omp_get_initial_device_num(); \
      if (omp_is_device(__cudaDev)) { \
        fprintf(stderr, "CUDA error at %s:%i : %s\n", file, line, cudaGetErrorString(cudaGetLastError())); \
        exit(1); \
      } \
    } \
  } while (0)

// Define a macro to check OpenACC errors in offloaded regions
#define __accCheckErrorOffload(file, line) \
  do { \
    int __err = omp_get_overall_verbosity(); \
    if (__err > 0 && !omp_in_parallel()) { \
      fprintf(stderr, "OpenACC error at %s:%i\n", file, line); \
      exit(1); \
    } else if (omp_is_initial_device() && __err == 2) { \
      omp_set_verbosity(0); \
      int err = acc_get_last_error(); \
      if (err != ACC_OK) { \
        fprintf(stderr, "OpenACC error at %s:%i : %s\n", file, line, acc_get_error_string(err)); \
        exit(1); \
      } \
    } else if (__err == 3) { \
      int __accDev = omp_get_initial_device_num(); \
      if (omp_is_device(__accDev)) { \
        fprintf(stderr, "OpenACC error at %s:%i : %s\n", file, line, acc_get_error_string(acc_get_last_error())); \
        exit(1); \
      } \
    } \
  } while (0)

// Define a macro to check OpenMP errors in offloaded regions
#define __ompCheckErrorOffload(file, line) \
  do { \
    int __err = omp_get_overall_verbosity(); \
    if (__err > 0 && !omp_in_parallel()) { \
      fprintf(stderr, "OpenMP error at %s:%i\n", file, line); \
      exit(1); \
    } else if (omp_is_initial_device() && __err == 2) { \
      omp_set_verbosity(0); \
      int err = omp_get_last_error(); \
      if (err != 0) { \
        fprintf(stderr, "OpenMP error at %s:%i : %s\n", file, line, omp_strerror(err)); \
        exit(1); \
      } \
    } else if (__err == 3) { \
      int __ompDev = omp_get_initial_device_num(); \
      if (omp_is_device(__ompDev)) { \
        fprintf(stderr, "OpenMP error at %s:%i : %s\n", file, line, omp_strerror(omp_get_last_error())); \
        exit(1); \
      } \
    } \
  } while (0)

// Macro to define offloaded regions
#define OFFLOAD_REGION(func) \
  void func() { \
    if (omp_is_initial_device()) { \
      __cudaCheckErrorCudaOffload(__FILE__, __LINE__); \
    } else { \
      __accCheckErrorOffload(__FILE__, __LINE__); \
      __ompCheckErrorOffload(__FILE__, __LINE__); \
    }

#endif