# CMakeLists.txt for SimpleMOC-kernel using Kokkos

cmake_minimum_required(VERSION 3.10)

# Set project name
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the executable
add_executable(SimpleMOC-kernel main.cpp kernel.cpp init.cpp io.cpp)

# Link Kokkos to the executable
target_link_libraries(SimpleMOC-kernel Kokkos::kokkos)

# Specify include directories
target_include_directories(SimpleMOC-kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set compilation options
if (Kokkos_ENABLE_CUDA)
    target_compile_definitions(SimpleMOC-kernel PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Optional: Set optimization flags
if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(SimpleMOC-kernel PRIVATE -O3)
endif()

# Optional: Add debugging flags if in Debug mode
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(SimpleMOC-kernel PRIVATE -g)
endif()

# Optional: Add profiling flags if requested
if (DEFINED PROFILE AND PROFILE)
    target_compile_options(SimpleMOC-kernel PRIVATE -pg)
endif()

# Optional: Add flags for exponential table if needed
if (DEFINED TABLE AND TABLE)
    target_compile_definitions(SimpleMOC-kernel PRIVATE TABLE)
endif()

# Set the output directory for the executable
set_target_properties(SimpleMOC-kernel PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})