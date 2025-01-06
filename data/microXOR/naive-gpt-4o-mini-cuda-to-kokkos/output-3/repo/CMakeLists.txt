cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add the executable
add_executable(microXOR main.cpp microXOR.cpp)

# Specify Kokkos as the backend
target_link_libraries(microXOR Kokkos::kokkos)

# Include directories
target_include_directories(microXOR PRIVATE include)

# Kokkos settings
if (Kokkos_ENABLE_CUDA)
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Add Kokkos parallel execution
target_compile_options(microXOR PRIVATE ${Kokkos_CXX_FLAGS})