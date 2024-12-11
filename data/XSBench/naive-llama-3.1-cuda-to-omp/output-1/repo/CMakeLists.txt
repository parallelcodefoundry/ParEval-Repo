cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
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

# Add OpenMP support
find_package(OpenMP REQUIRED)
include_directories(${OPENMP_INCLUDE_DIR})
link_directories(${OPENMP_LIBRARIES})

add_executable(XSBench ${SOURCE})

target_link_libraries(XSBench ${OPENMP_LIBRARIES})

install(TARGETS XSBench DESTINATION bin)

# Set OpenMP flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Add offload target to CUDA devices
set_target_properties(XSBench PROPERTIES OFFLOAD_DEVICES "nvidia")