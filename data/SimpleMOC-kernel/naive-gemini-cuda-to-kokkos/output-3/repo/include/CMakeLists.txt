cmake_minimum_required(VERSION 3.10)
project(SimpleMOC-kernel)

# Set compiler and other options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_CXX_COMPILER MATCHES "clang\\+\\+")
  message(WARNING "Recommended to use clang++ as the host compiler")
endif

# Kokkos configuration
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif
option(KOKKOS_BACKEND "Kokkos backend (Serial, OpenMP, CUDA, OpenCL, SYCL)" "CUDA")
if (KOKKOS_BACKEND STREQUAL "CUDA")
  find_package(CUDA REQUIRED)
endif
find_package(Kokkos REQUIRED)
add_definitions(${Kokkos_DEFINITIONS})

# Add include directories
include_directories(include)

# Source files
set(SOURCES
  main.cpp
  kernel.cpp
  init.cpp
  io.cpp
)


# Add executables
add_executable(SimpleMOC-kernel ${SOURCES})
target_link_libraries(SimpleMOC-kernel ${Kokkos_LIBRARIES} ${CUDA_LIBRARIES})

# Set build options based on user settings
if (OPTIMIZE STREQUAL "yes")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
endif

if (DEBUG STREQUAL "yes")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif

if (TABLE STREQUAL "yes")
    add_definitions(-DTABLE)
endif


# Add a custom target to generate the exponential table if needed
# (this part is not strictly necessary but demonstrates how one might
# handle the TABLE flag in CMake).  The original Makefile included
# TABLE handling in the compilation flags.  For Kokkos, it's often
# easier to handle such compilation-time switches using `add_definitions`.
# The Table is now generated in init.cpp as an array rather than as a
# separate function.

#Example for separate table generation:
#if (TABLE STREQUAL "yes")
# add_custom_target(generate_table
#  COMMAND ${CMAKE_COMMAND} -E echo "Generating Exponential Table..."
#  COMMAND ${CMAKE_COMMAND} -E touch table.dat # replace with your actual table generation command
# )
# add_dependencies(SimpleMOC-kernel generate_table)
#endif
