# CMakeLists.txt for SimpleMOC-kernel

cmake_minimum_required(VERSION 3.10)

# Project name
project(SimpleMOC-kernel)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(SimpleMOC-kernel
    main.cpp
    kernel.cpp
    init.cpp
    io.cpp
)

# Link Kokkos to the executable
target_link_libraries(SimpleMOC-kernel Kokkos::kokkos)

# Include directories
target_include_directories(SimpleMOC-kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set Kokkos options
if (Kokkos_ENABLE_CUDA)
    target_compile_definitions(SimpleMOC-kernel PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Optional: Set optimization flags
if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(SimpleMOC-kernel PRIVATE -O3)
endif()

# Optional: Add debugging options
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(SimpleMOC-kernel PRIVATE -g)
endif()

# Optional: Add profiling options
if (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    target_compile_options(SimpleMOC-kernel PRIVATE -pg)
endif()