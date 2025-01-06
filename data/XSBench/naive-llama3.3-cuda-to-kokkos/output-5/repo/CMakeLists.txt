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

# Target properties
target_include_directories(XSBench PUBLIC ${Kokkos_Core_INCLUDES})
target_link_libraries(XSBench ${Kokkos_Core_LIBS})

# Install the executable
install(TARGETS XSBench DESTINATION bin)