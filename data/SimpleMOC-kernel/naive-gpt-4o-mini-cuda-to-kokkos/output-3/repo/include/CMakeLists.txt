# CMakeLists.txt for SimpleMOC-kernel using Kokkos

cmake_minimum_required(VERSION 3.10)

# Set project name
project(SimpleMOC-kernel)

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(SimpleMOC-kernel main.cpp kernel.cpp init.cpp io.cpp)

# Link Kokkos to the executable
target_link_libraries(SimpleMOC-kernel Kokkos::kokkos)

# Include directories
target_include_directories(SimpleMOC-kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set Kokkos options
if (Kokkos_ENABLE_CUDA)
    target_compile_definitions(SimpleMOC-kernel PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Set optimization flags
if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(SimpleMOC-kernel PRIVATE -O3)
endif()

# Add custom commands for building and running
add_custom_target(run
    COMMAND SimpleMOC-kernel
    DEPENDS SimpleMOC-kernel
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Optionally, you can add a clean target
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove SimpleMOC-kernel
    COMMENT "Cleaning build directory and executable"
)