cmake_minimum_required(VERSION 3.16)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(Kokkos REQUIRED)

if (${KOKKOS_BACK_END} STREQUAL "CUDA")
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)

    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -extended-lambda -Wext-lambda-captures-this -expt-relaxed-constexpr")

    set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CUDA)
elseif (${KOKKOS_BACK_END} STREQUAL "HIP")
    enable_language(HIP)
    set(CMAKE_HIP_STANDARD 17)

    set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE HIP)
endif ()


set(SOURCE Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)

add_executable(XSBench ${SOURCE})
target_link_libraries(XSBench Kokkos::kokkos)

install(TARGETS XSBench DESTINATION bin)
