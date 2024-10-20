cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP_Offload
    VERSION 1.0
    LANGUAGES CXX CUDA
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -Xopenmp-target=nvptx64-nvidia-cuda" )

set(SOURCE Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu)

set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CXX)

add_executable(XSBench ${SOURCE})

target_link_libraries(XSBench  ${OpenMP_CXX_FLAGS})
install(TARGETS XSBench DESTINATION bin)