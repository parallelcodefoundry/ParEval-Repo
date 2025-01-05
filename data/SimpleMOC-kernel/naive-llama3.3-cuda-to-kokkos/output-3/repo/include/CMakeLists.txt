cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find the Kokkos library
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the source files
set(SOURCES
  kernel.cpp
  main.cpp
  init.cpp
  io.cpp
)

# Add the header files
set(HEADERS
  SimpleMOC-kernel_header.hpp
)

# Create the executable
add_executable(SimpleMOC-kernel ${SOURCES})

# Link the Kokkos library
target_link_libraries(SimpleMOC-kernel Kokkos::KokkosCore)

# Add include directories
target_include_directories(SimpleMOC-kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add compiler flags
target_compile_options(SimpleMOC-kernel PUBLIC -Wall -Wextra)

# Add the Kokkos options
target_compile_definitions(SimpleMOC-kernel PUBLIC KOKKOS_ENABLE_CUDA_LAMBDA)
target_compile_options(SimpleMOC-kernel PUBLIC ${Kokkos_CXX_FLAGS})