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

# Kokkos specific include directories
target_include_directories(XSBench PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  ${Kokkos_INCLUDE_DIRS}
)


# list of source files
set(SOURCE Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)
add_executable(XSBench ${SOURCE} XSbench_header.hpp XSbench_shared_header.h)

# Link Kokkos libraries
target_link_libraries(XSBench ${Kokkos_LIBRARIES})


install(TARGETS XSBench DESTINATION bin)
