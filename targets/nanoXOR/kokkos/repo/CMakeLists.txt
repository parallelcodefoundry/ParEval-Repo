cmake_minimum_required(VERSION 3.16)

project(
    nanoXOR_Kokkos
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

if (${KOKKOS_BACKEND} STREQUAL "CUDA")
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)

    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -extended-lambda -Wext-lambda-captures-this -expt-relaxed-constexpr")

    set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CUDA)
elseif (${KOKKOS_BACKEND} STREQUAL "HIP")
    enable_language(HIP)
    set(CMAKE_HIP_STANDARD 17)

    set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE HIP)
endif ()


set(SOURCE nanoXOR.cpp)

add_executable(nanoXOR.exe ${SOURCE})
target_link_libraries(nanoXOR.exe Kokkos::kokkos)

install(TARGETS nanoXOR.exe DESTINATION bin)
