cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OPENMP
    VERSION 1.0
    LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# OpenMP flags
find_package(OpenMP REQUIRED)
include_directories(${OPENMP_INCLUDE_DIR})
link_directories(${OPENMP_LIBRARY_DIRS})

set(SOURCE Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu)

set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CXX)

add_executable(XSBench ${SOURCE})

target_link_libraries(XSBench ${OPENMP_LIBRARIES})

# Offloading flags
set(CMAKE_OFFLOAD_ARCHITECTURES "nvidia")
set_target_properties(XSBench PROPERTIES OFFLOAD_COMPILE_COMMAND "${CMAKE_CXX_COMPILER} -Xopenmp --offload-architecture=nvidia")

install(TARGETS XSBench DESTINATION bin)