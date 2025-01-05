cmake_minimum_required(VERSION 3.10)
project(microXOR)

set(CMAKE_CXX_STANDARD 17)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Add include directories
include_directories(include)

# Set Kokkos backend (CUDA in this example)
# This can be changed to Serial, OpenMP, OpenCL, or other supported backends
set(KOKKOS_BACKEND CUDA)

# Source files
set(SOURCES
    src/main.cpp
    src/microXOR.cpp
)

# Add Kokkos libraries
target_link_libraries(microXOR Kokkos::KokkosCore)

# Add executable
add_executable(microXOR ${SOURCES})

# Set compiler flags (optional)
# target_compile_options(microXOR PRIVATE -O3) # For optimization. Remove for debugging

# CUDA specific settings (if using CUDA backend)
if(${KOKKOS_BACKEND} STREQUAL "CUDA")
  # Add CUDA include directory (adapt path if necessary)
  include_directories(/usr/local/cuda/include) # Replace with your CUDA installation path
  target_link_libraries(microXOR cudart) # Link CUDA runtime library

  # Add arch flag for CUDA (adjust as needed)
  set(CMAKE_CUDA_ARCHITECTURES 80) # Replace with your target architecture
endif()