cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu)

include_directories(/usr/include/openmp)
link_directories(/usr/lib/x86_64-linux-gnu/omp-runtime/libopenmpi.so)

add_executable(XSBench ${SOURCE})

target_link_libraries(XSBench openmpi libgomp)

set_property(TARGET XSBench PROPERTY OFFLOAD_ARCH x86-64-nvidia)