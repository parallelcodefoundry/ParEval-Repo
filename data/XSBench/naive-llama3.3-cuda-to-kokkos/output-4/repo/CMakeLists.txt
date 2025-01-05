cmake_minimum_required(VERSION 3.10)
project(XSBench_Kokkos)

# Find the Kokkos core package
find_package(Kokkos REQUIRED)

# Add the Kokkos CUDA backend
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CUDA_COMPILER nvcc)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the source files
set(SOURCE
  Main.cpp
  io.cpp
  Simulation.cpp
  GridInit.cpp
  XSutils.cpp
  Materials.cpp
)

# Add the header files
set(HEADERS
  XSbench_shared_header.hpp
  XSbench_header.hpp
)

# Create the executable
add_executable(XSBench ${SOURCE})

# Link the Kokkos library
target_link_libraries(XSBench Kokkos::KokkosCore)

# Add include directories
target_include_directories(XSBench PUBLIC ${Kokkos_INCLUDE_DIRS})

# Set the CUDA architecture
set(CUDA_ARCHITECTURE "60" CACHE STRING "CUDA architecture")
set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCHITECTURE})

# Set the C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Wall -Wextra")

# Install the executable
install(TARGETS XSBench DESTINATION bin)