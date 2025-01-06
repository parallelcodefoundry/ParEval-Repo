cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

# Set the Kokkos backend
find_package(Kokkos REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Specify the source files
set(SOURCE 
    Main.cpp 
    io.cpp 
    Simulation.cpp 
    GridInit.cpp 
    XSutils.cpp 
    Materials.cpp
)

# Create an executable
add_executable(XSBench ${SOURCE})

# Link Kokkos to the executable
target_link_libraries(XSBench Kokkos::kokkos)

# Include directories for Kokkos
target_include_directories(XSBench PRIVATE ${Kokkos_INCLUDE_DIRS})

# Set Kokkos options
set_target_properties(XSBench PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

# Enable Kokkos features
if(Kokkos_ENABLE_CUDA)
    target_compile_definitions(XSBench PRIVATE KOKKOS_ENABLE_CUDA)
endif()

# Add any additional Kokkos settings here