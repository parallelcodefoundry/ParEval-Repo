cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add the executable
add_executable(microXOR main.cpp microXOR.cpp)

# Link Kokkos to the executable
target_link_libraries(microXOR Kokkos::kokkos)

# Specify the Kokkos backend (CUDA)
if (Kokkos_CXX11_COMPILER)
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Set Kokkos options
set_target_properties(microXOR PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Enable Kokkos parallel execution
Kokkos_add_library(microXOR_lib
    SOURCES microXOR.cpp
    HEADERS microXOR.hpp
    OPTIONS
        Kokkos_ENABLE_CUDA
)

# Add the executable that uses the Kokkos library
add_executable(microXOR main.cpp)
target_link_libraries(microXOR PRIVATE microXOR_lib)