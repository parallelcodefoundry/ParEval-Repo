cmake_minimum_required(VERSION 3.21)

project(
    XSBench_Kokkos
    VERSION 1.0
    LANGUAGES CXX
)

# Find Kokkos package
find_package(Kokkos REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCE 
    Main.cpp 
    io.cpp 
    Simulation.cpp 
    GridInit.cpp 
    XSutils.cpp 
    Materials.cpp
)

# Create executable
add_executable(XSBench ${SOURCE})

# Link Kokkos to the executable
target_link_libraries(XSBench Kokkos::kokkos)

# Set Kokkos options
target_compile_definitions(XSBench PRIVATE KOKKOS_ENABLE_CUDA)
target_compile_options(XSBench PRIVATE ${Kokkos_CXX_FLAGS})

# Set include directories
target_include_directories(XSBench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Optional: Add installation rules
install(TARGETS XSBench DESTINATION bin)