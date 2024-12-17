cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OPENMP
    VERSION 1.0
    LANGUAGES CXX
    OFFLOAD_TARGETS nvidia, host
    OMPTargets CUDA
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c)

set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE C)
set_property(TARGET ${TARGET} PROPERTY OMPCudaDeviceFlags -arch=sm_80)

add_executable(XSBench ${SOURCE})

install(TARGETS XSBench DESTINATION bin)

target_link_libraries( XSBench OPENMP::OFFLOAD )