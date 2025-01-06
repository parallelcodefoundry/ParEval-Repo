cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler and architecture flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17") #Needed for C++17 features
if(KOKKOS_BACKEND STREQUAL "CUDA")
  set(CMAKE_CUDA_ARCHITECTURES 35) # Adjust as needed
  # Add CUDA specific flags if necessary
endif

# Specify source files
set(SOURCES
  main.cpp
  kernel.cpp
  init.cpp
  io.cpp
)

# Add header file directory
include_directories(./)

# Add Kokkos library to the link libraries
target_link_libraries(SimpleMOC-kernel Kokkos::Kokkos)

# Create executable
add_executable(SimpleMOC-kernel ${SOURCES} SimpleMOC-kernel_header.hpp)

# Set options (OPTIMIZE, DEBUG, PROFILE, TABLE)
option(OPTIMIZE "Enable compiler optimizations" ON)
option(DEBUG "Enable debugging information" OFF)
option(PROFILE "Enable profiling" OFF)
option(TABLE "Use exponential table" OFF)


if(OPTIMIZE)
  target_compile_options(SimpleMOC-kernel PRIVATE -O3 -ffast-math)
endif

if(DEBUG)
  target_compile_options(SimpleMOC-kernel PRIVATE -g)
endif

if(PROFILE)
  target_compile_options(SimpleMOC-kernel PRIVATE -pg)
endif

if(TABLE)
  target_compile_options(SimpleMOC-kernel PRIVATE -DTABLE)
endif

# Add custom commands if necessary (e.g., for pre-processing steps)

