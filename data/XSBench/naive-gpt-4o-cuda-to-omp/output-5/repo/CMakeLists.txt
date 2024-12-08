cmake_minimum_required(VERSION 3.21)

project(
    XSBench_OpenMP
    VERSION 1.0
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Add OpenMP flags
find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(SOURCE Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu)

# Change file extensions from .cu to .cpp for OpenMP offloading
set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CXX)

add_executable(XSBench ${SOURCE})

# Link OpenMP
target_link_libraries(XSBench PRIVATE OpenMP::OpenMP_CXX)

install(TARGETS XSBench DESTINATION bin)