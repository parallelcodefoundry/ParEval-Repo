cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES CXX OFFLOAD

)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu)

add_executable(XSBench ${SOURCE})

target_link_libraries(XSBench OpenMP::OpenMP_CXX_OFFLOAD)

# Define offload targets
set_target_properties(XSBench PROPERTIES OFFLOAD_TARGETS nvidia)

include_directories(/usr/local/cuda/include /usr/lib/x86_64-linux-gnu/openmp)