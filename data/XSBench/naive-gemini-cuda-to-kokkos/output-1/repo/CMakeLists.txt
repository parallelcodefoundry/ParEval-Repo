cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find Kokkos
find_package(Kokkos REQUIRED)

# Kokkos specific flags.  These might need adjustments depending on your Kokkos installation
set(KOKKOS_FLAGS "${Kokkos_CXX_FLAGS} ${Kokkos_LINK_FLAGS}")


set(SOURCE Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)
add_executable(XSBench ${SOURCE} XSbench_header.hpp XSbench_shared_header.h)

target_link_libraries(XSBench Kokkos::Kokkos) #Link against Kokkos library

# Set compiler and linker flags
target_compile_options(XSBench PRIVATE ${CMAKE_CXX_FLAGS} ${KOKKOS_FLAGS})
target_link_options(XSBench PRIVATE ${KOKKOS_FLAGS})


install(TARGETS XSBench DESTINATION bin)