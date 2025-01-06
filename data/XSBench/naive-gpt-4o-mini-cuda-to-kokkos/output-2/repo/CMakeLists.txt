cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

# Set the Kokkos backend
set(KOKKOS_BACKEND "CUDA" CACHE STRING "Kokkos backend to use")
set_property(CACHE KOKKOS_BACKEND PROPERTY STRINGS "CUDA" "OpenMP" "Threads")

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCE
    Main.cpp
    io.cpp
    Simulation.cpp
    GridInit.cpp
    XSutils.cpp
    Materials.cpp
)

# Create executable
add_executable(XSBench ${SOURCE})

# Link Kokkos to the executable
target_link_libraries(XSBench Kokkos::kokkos)

# Include Kokkos headers
target_include_directories(XSBench PRIVATE ${Kokkos_INCLUDE_DIRS})

# Set compile options
target_compile_options(XSBench PRIVATE
    -Wall
    -Wextra
)

# Set optimization flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(XSBench PRIVATE -g)
else()
    target_compile_options(XSBench PRIVATE -O3)
endif()

# Install target
install(TARGETS XSBench DESTINATION bin)