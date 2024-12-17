cmake_minimum_required(VERSION 3.21)

project(
    XSBench
    VERSION 1.0
    LANGUAGES CXX OPENMP
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c)

set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE C)

add_executable(XSBench ${SOURCE})

install(TARGETS XSBench DESTINATION bin)