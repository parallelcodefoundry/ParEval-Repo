cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# Add the executable
add_executable(XSBench Main.cpp io.cpp Simulation.cpp GridInit.cpp XSutils.cpp Materials.cpp)

# Link Kokkos to the executable
target_link_libraries(XSBench Kokkos::kokkos)

# Set Kokkos options
set_target_properties(XSBench PROPERTIES
    Kokkos_ENABLE_CUDA ON
    Kokkos_ENABLE_OPENMP OFF
)

# Include directories
target_include_directories(XSBench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install target
install(TARGETS XSBench DESTINATION bin)