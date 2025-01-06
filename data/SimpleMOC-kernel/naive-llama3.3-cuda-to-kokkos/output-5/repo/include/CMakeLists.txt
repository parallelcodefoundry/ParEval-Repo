cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Add Kokkos flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Kokkos_CXX_FLAGS}")

# Add source files
set(SOURCES
  kernel.cpp
  main.cpp
  init.cpp
  io.cpp
)

# Add header files
set(HEADERS
  SimpleMOC-kernel_header.hpp
)

# Create executable
add_executable(SimpleMOC-kernel ${SOURCES} ${HEADERS})

# Link Kokkos
target_link_libraries(SimpleMOC-kernel ${Kokkos_LIBS})

# Set CUDA architecture
if (KOKKOS_BACKEND STREQUAL "CUDA")
  set(CUDA_ARCHITECTURE "52" CACHE STRING "CUDA architecture")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch=sm_${CUDA_ARCHITECTURE}")
endif()