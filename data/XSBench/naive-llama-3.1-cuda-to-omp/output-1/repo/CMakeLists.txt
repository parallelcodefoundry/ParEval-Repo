cmake_minimum_required(VERSION 3.10)
project(XSBench)

# Enable OpenMP
find_package(OpenMP REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    # Offload support
    add_definitions(-D__INTEL_OFFLOAD_SUPPORTED__)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp XSBSolver.cpp ${XSBSOLVER_SOURCE})

target_link_libraries(${PROJECT_NAME}
                      ${OpenMP_LIBRARIES}
                      mkl_intel_lp64
                      mkl_sequential
                      mkl_core
                      mkl_tbb_thread
                     )

# OpenMP flags for offloading support
if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    target_compile_options(${PROJECT_NAME} PUBLIC /Qoffload:device=${OFFLOAD_DEVICE})
endif()

target_link_libraries(${PROJECT_NAME}
                      mkl_intel_lp64
                      mkl_sequential
                      mkl_core
                      mkl_tbb_thread
                     )

# Offloading options for Intel compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(OFFLOAD_DEVICE ${OFFLOAD_DEVICE} CACHE STRING "Offload device to use")
endif()

add_dependencies(${PROJECT_NAME} ${XSBSOLVER_DEPENDENCIES})

target_include_directories(${PROJECT_NAME}
                            PUBLIC ${XSBSOLVER_INCLUDE_DIR})