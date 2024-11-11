cmake_minimum_required(VERSION 3.21)

project(
    XSBench_CUDA
    VERSION 1.0
    LANGUAGES CXX CUDA
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu)

set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CUDA)

add_executable(XSBench ${SOURCE})

install(TARGETS XSBench DESTINATION bin)
