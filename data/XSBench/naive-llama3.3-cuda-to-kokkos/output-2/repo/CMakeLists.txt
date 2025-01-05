cmake_minimum_required(VERSION 3.10)
project(XSBench_Kokkos)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set up the compiler
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the Kokkos backend
set(KOKKOS_BACKEND CUDA)

# Add the Kokkos library to the project
add_executable(XSBench_Kokkos
  Main.cpp
  io.cpp
  Simulation.cpp
  GridInit.cpp
  XSutils.cpp
  Materials.cpp
)

# Link the Kokkos library
target_link_libraries(XSBench_Kokkos Kokkos::Kokkos)

# Set the include directories
target_include_directories(XSBench_Kokkos PUBLIC
  ${KOKKOS_INCLUDE_DIRS}
)

# Set the compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Install the executable
install(TARGETS XSBench_Kokkos DESTINATION bin)