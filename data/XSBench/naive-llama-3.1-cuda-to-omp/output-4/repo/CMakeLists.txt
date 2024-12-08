# Copyright (c) 2023, The Regents of the University of Chicago
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Regents of the University of Chicago nor the names
#    of its contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.10)

project(XSBench OFFLINE=ON)

add_definitions(-DOPENMP)
find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set up OpenMP offloading
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.16)
  set(OFFLOAD_TARGETS "omp:offload")
else()
  set(OFFLOAD_TARGETS "openmp")
endif()

set_target_properties(XSBench PROPERTIES
  OFFLOAD_COMPILE_OPTIONS ""
  OFFLOAD_LINK_OPTIONS "")
add_executable(${PROJECT_NAME} main.cpp
  ${XS_BENCH_SRCS}
  ${XS_BENCH_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.16)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
    OpenMP::Offload)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
    ${OPENMP_LIBRARIES})
endif()

# Offloading rules for main.cpp
set_target_properties(main PROPERTIES
  OFFLOAD_COMPILE_OPTIONS ""
  OFFLOAD_LINK_OPTIONS "")

target_sources(${PROJECT_NAME} PRIVATE main.cpp)

# Rule for offloading XSBench_shared_header.h
add_library(XS_BENCH_SHARED_PUBLIC INTERFACE)
add_library(XS_BENCH_SHARED_PRIVATE INTERFACE)

target_sources(XS_BENCH_SHARED_PUBLIC PRIVATE ${XS_BENCH_HEADERS})

target_sources(XS_BENCH_SHARED_PRIVATE PRIVATE XSbench_shared_header.h)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Generate OpenMP offloading rules for XSBench
foreach(LIBRARY ${XS_BENCH_LIBRARIES})
  add_library(${LIBRARY}_OFFLOAD INTERFACE)
  target_sources(${LIBRARY}_OFFLOAD PRIVATE
    ${XS_BENCH_SHARED_PRIVATE}/XSbench_shared_header.h
    ${XS_BENCH_HEADERS}/${LIBRARY}.cpp)
endforeach()

# Link OpenMP offloading rules for XSBench
foreach(LIBRARY ${XS_BENCH_LIBRARIES})
  add_library(${LIBRARY} ${LIBRARY}_OFFLOAD OBJECTS)

  target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBRARY}
    Threads::Threads
    OpenMP::Offload)
endforeach()

# Generate OpenMP offloading rules for XSbench_shared_header.h
add_library(XSBENCH_SHARED OFFLINE)
target_sources(XSBENCH_SHARED PRIVATE
  ${XS_BENCH_HEADERS}/XSbench_shared_header.h)

foreach(LIBRARY ${XS_BENCH_LIBRARIES})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARY})
endforeach()

# Link OpenMP offloading rules for XSBench
add_library(XSBENCH ${XS_BENCH_SHARED} OBJECTS)
target_link_libraries(${PROJECT_NAME} PRIVATE
  XSBENCH
  Threads::Threads
  OpenMP::Offload)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin)

# Copy offloaded code for OpenMP library
foreach(LIBRARY ${XS_BENCH_LIBRARIES})
  add_custom_command(TARGET ${LIBRARY} POST_BUILD
    COMMAND cp -r "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY}_OFFLOAD/lib${LIBRARY}.a" "$<TARGET_FILE_DIR:${LIBRARY}>/lib")
endforeach()