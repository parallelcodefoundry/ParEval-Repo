cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

# Find Kokkos
find_package(Kokkos REQUIRED)

# Set compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Kokkos specific flags.  Adjust as needed for your system and Kokkos version.
set(KOKKOS_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Source files
set(SOURCE
    Main.cpp
    io.cpp
    Simulation.cpp
    GridInit.cpp
    XSutils.cpp
    Materials.cpp
    )

# Header files
set(HEADER
    XSbench_header.hpp
    XSbench_shared_header.h
    )

# Add executable
add_executable(XSBench ${SOURCE})

# Link Kokkos libraries
target_link_libraries(XSBench Kokkos::Kokkos)

# Install target (optional)
install(TARGETS XSBench DESTINATION bin)

# Add include directories. Adjust paths if necessary.  Make sure to include Kokkos headers.
target_include_directories(XSBench PUBLIC
  .
  ${Kokkos_INCLUDE_DIRS} #This should automatically pick the right path from find_package(Kokkos)
  )

