cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add the executable
add_executable(microXOR main.cpp microXOR.cpp)

# Link Kokkos to the executable
target_link_libraries(microXOR Kokkos::kokkos)

# Specify the Kokkos backend
if (DEFINED ENV{KOKKOS_BACKEND})
    set(KOKKOS_BACKEND $ENV{KOKKOS_BACKEND})
else()
    message(FATAL_ERROR "Please set the KOKKOS_BACKEND environment variable.")
endif()

# Set Kokkos options
if (KOKKOS_BACKEND STREQUAL "CUDA")
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_CUDA)
elseif (KOKKOS_BACKEND STREQUAL "OpenMP")
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_OPENMP)
elseif (KOKKOS_BACKEND STREQUAL "Serial")
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_SERIAL)
else()
    message(FATAL_ERROR "Unsupported KOKKOS_BACKEND: ${KOKKOS_BACKEND}")
endif()

# Include Kokkos headers
target_include_directories(microXOR PRIVATE ${Kokkos_INCLUDE_DIRS})

# Set the build type
set(CMAKE_BUILD_TYPE Release)