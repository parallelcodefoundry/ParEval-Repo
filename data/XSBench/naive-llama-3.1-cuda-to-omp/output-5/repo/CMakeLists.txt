cmake_minimum_required(VERSION 3.10)
project(XSBench)

# Set up OpenMP and Offloading
set(CMAKE_CXX_COMPILER "omp++")
set(CMAKE_C_COMPILER "ompcc")

find_package(OpenMP REQUIRED)
find_package(MetaOpenACC REQUIRED)

# Set up compile flags for offload to GPU
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
  set(OFFLOAD_COMPILE_FLAGS "-fopenacc -fopenmp-targets=nvptx64-nvidia-cuda11.0")
else()
  set(OFFLOAD_COMPILE_FLAGS "-fopenmp-targets=gpu --cuda-gpu-arch=compute_70")
endif()

# Set up compile flags for OpenMP
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OFFLOAD_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OFFLOAD_COMPILE_FLAGS}")

# Include the shared header file
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared_header.h)

# Define source files and executables
add_executable(XSBench main.cpp ${XSBENCH_SOURCE_FILES})
target_link_libraries(XSBench ${OpenMP_LIBRARIES})

# Set up offload flags for each executable
foreach(exec XSBench)
  set_target_properties(${exec} PROPERTIES OFFLOAD_COMPILE_FLAGS "${OFFLOAD_COMPILE_FLAGS}")
endforeach()

# Define custom build rule to print timing information at the end of each run
add_custom_command(TARGET XSBench POST_BUILD
                   COMMAND ${CMAKE_CXX_COMPILER} ${XSBENCH_SOURCE_FILES} -o $<TARGET_FILE:XSBL> --offload-architecture=gpu --offload-exec=xsbench_gnu
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${XSBENCH_SOURCE_FILES})

# Print timing information at the end of each run
add_custom_command(TARGET XSBench POST_BUILD
                   COMMAND ${CMAKE_CXX_COMPILER} ${XSBENCH_SOURCE_FILES} -o $<TARGET_FILE:XSBL> --offload-architecture=gpu --offload-exec=xsbench_gnu
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND ${CMAKE_CXX_COMPILER} ${XSBENCH_SOURCE_FILES} -o xsbench_host.exe
                   COMMAND ${CMAKE_CXX_COMPILER} ${XSBENCH_SOURCE_FILES} -o $<TARGET_FILE:XSBL> --offload-architecture=gpu --offload-exec=xsbench_gnu
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${XSBENCH_SOURCE_FILES})

# Print profiling information at the end of each run
add_custom_command(TARGET XSBench POST_BUILD
                   COMMAND ${CMAKE_CXX_COMPILER} ${XSBENCH_SOURCE_FILES} -o $<TARGET_FILE:XSBL> --offload-architecture=gpu --offload-exec=xsbench_gnu
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND print_profile(${PROFILE}, ${in})
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${XSBENCH_SOURCE_FILES})

# Print profiling information at the end of each run (alternative method)
add_custom_command(TARGET XSBench POST_BUILD
                   COMMAND ${CMAKE_CXX_COMPILER} ${XSBENCH_SOURCE_FILES} -o $<TARGET_FILE:XSBL> --offload-architecture=gpu --offload-exec=xsbench_gnu
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND profile_print(${PROFILE}, ${in})
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${XSBENCH_SOURCE_FILES})

# Define custom build rule to run the executable and collect timing information
add_custom_command(TARGET XSBench PRE_BUILD
                   COMMAND ${CMAKE_CXX_COMPILER} xsbench_host.exe --offload-architecture=gpu --offload-exec=xsbench_gnu
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${XSBENCH_SOURCE_FILES})