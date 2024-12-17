cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES C
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# Add OpenMP flags
find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

set(SOURCE Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c)

add_executable(XSBench ${SOURCE})

# Link OpenMP
target_link_libraries(XSBench PRIVATE OpenMP::OpenMP_C)

install(TARGETS XSBench DESTINATION bin)