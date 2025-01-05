cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find Kokkos
find_package(Kokkos REQUIRED)

# Kokkos-specific include directories.  May need adjustments based on your Kokkos installation
# include_directories(${KOKKOS_INCLUDE_DIRS})


set(SOURCE Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)
set(HEADER XSbench_header.hpp XSbench_shared_header.h)


add_executable(XSBench ${SOURCE} ${HEADER})

target_link_libraries(XSBench Kokkos::Kokkos)


install(TARGETS XSBench DESTINATION bin)