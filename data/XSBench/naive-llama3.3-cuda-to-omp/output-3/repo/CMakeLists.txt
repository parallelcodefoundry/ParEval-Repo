cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP_Offload
    VERSION 1.0
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# OpenMP-offload flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda")

set(SOURCE Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)

add_executable(XSBench ${SOURCE})

install(TARGETS XSBench DESTINATION bin)