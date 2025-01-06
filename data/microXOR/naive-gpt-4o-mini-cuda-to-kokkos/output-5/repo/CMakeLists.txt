cmake_minimum_required(VERSION 3.10)
project(microXOR)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(microXOR main.cpp microXOR.cpp)

# Specify Kokkos as the backend for the target
target_link_libraries(microXOR Kokkos::kokkos)

# Include directories
target_include_directories(microXOR PRIVATE include)

# Kokkos settings
if(Kokkos_ENABLE_CUDA)
    target_compile_definitions(microXOR PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Set Kokkos options
set_target_properties(microXOR PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

# Add a custom command to run the executable
add_custom_command(TARGET microXOR POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Run the executable with: ./microXOR <N> <blockEdge>"
)