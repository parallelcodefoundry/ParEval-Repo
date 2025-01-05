cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler flags
set(CMAKE_CXX_STANDARD 17)
if (DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
  # Add debugging flags for Kokkos (if necessary)
else()
  set(CMAKE_BUILD_TYPE Release)
  # Add optimization flags for Kokkos (if necessary)
endif()


# Add include directories
include_directories(include)

# Add source files
add_executable(microXOR src/main.cpp src/microXOR.cpp)

# Link against Kokkos libraries
target_link_libraries(microXOR Kokkos::Kokkos)

# Add CUDA specific flags if Kokkos backend is CUDA
if(KOKKOS_BACKEND STREQUAL "CUDA")
  set(CMAKE_CUDA_ARCHITECTURES 80) # Replace with your target architecture
  target_compile_options(microXOR PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_80>) # Add CUDA architecture flag
endif()
