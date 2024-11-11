cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP_Offload
    VERSION 1.0
    LANGUAGES C
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(SOURCE Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c)

add_executable(XSBench ${SOURCE})

separate_arguments(OFFLOAD_FLAGS)
target_compile_options(XSBench PUBLIC "$<$<CONFIG:RELEASE>:${OFFLOAD_FLAGS}>")
target_link_options(XSBench PUBLIC "$<$<CONFIG:RELEASE>:${OFFLOAD_FLAGS}>")

target_link_libraries(XSBench PUBLIC m)

install(TARGETS XSBench DESTINATION bin)
