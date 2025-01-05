cmake_minimum_required(VERSION 3.10)
project(microXOR)

set(CMAKE_CXX_STANDARD 17)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Add include directories
include_directories(include)

# Add source files
add_executable(microXOR src/main.cpp src/microXOR.cpp)

# Link Kokkos libraries
target_link_libraries(microXOR Kokkos::Kokkos)


# Set compiler flags (optional, but recommended)
# Example for CUDA backend:
if(KOKKOS_BACKEND STREQUAL "CUDA")
  target_compile_options(microXOR PRIVATE -O3) # Or -g for debugging

  # Add CUDA specific flags if needed
  #  target_compile_options(microXOR PRIVATE $<$<CUDA_COMPILER_ID:NVCC>:-arch=sm_80>)
endif()

# Example for OpenMP backend:
if(KOKKOS_BACKEND STREQUAL "OpenMP")
  target_compile_options(microXOR PRIVATE -O3 -fopenmp)
  target_link_libraries(microXOR PRIVATE  gomp)
endif()

# Example for Serial backend:
if(KOKKOS_BACKEND STREQUAL "Serial")
  target_compile_options(microXOR PRIVATE -O3)
endif()
