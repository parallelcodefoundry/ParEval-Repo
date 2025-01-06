cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add executable
add_executable(microXOR main.cpp microXOR.cpp)

# Link Kokkos to the executable
target_link_libraries(microXOR Kokkos::kokkos)

# Specify Kokkos options
if (Kokkos_ENABLE_CUDA)
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Include directories
target_include_directories(microXOR PRIVATE include)

# Set Kokkos execution space
set_target_properties(microXOR PROPERTIES
    Kokkos_EXECUTION_SPACE "CUDA"
)

# Add build options
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(microXOR PRIVATE -g -G)
else()
    target_compile_options(microXOR PRIVATE -O3)
endif()