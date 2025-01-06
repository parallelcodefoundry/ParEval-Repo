# CMakeLists.txt for SimpleMOC-kernel using Kokkos

cmake_minimum_required(VERSION 3.10)

# Project name
project(SimpleMOC-kernel)

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the source files
set(SOURCES
    main.cpp
    kernel.cpp
    init.cpp
    io.cpp
)

# Create an executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link Kokkos to the executable
target_link_libraries(${PROJECT_NAME} Kokkos::kokkos)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Enable Kokkos features
if(Kokkos_ENABLE_CUDA)
    target_compile_definitions(${PROJECT_NAME} PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Set compiler options for Kokkos
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall)
endif()

# Add any additional compile definitions or flags as needed