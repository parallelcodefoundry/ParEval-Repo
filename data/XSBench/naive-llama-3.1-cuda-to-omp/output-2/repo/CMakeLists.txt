cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES CXX CUDA
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Enable OpenMP for offloading
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS "-fopenmp")

# Include necessary compiler flags and libraries
include_directories(/usr/include/offload)

add_executable(XSBench ${SOURCE})

target_link_libraries(XSBench pthread openmp offline-capable cuda)

install(TARGETS XSBench DESTINATION bin)