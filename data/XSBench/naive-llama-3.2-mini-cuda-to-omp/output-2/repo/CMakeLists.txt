# Set OpenMP library for OpenMP-offload compiler flag (-fopenmp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Enable OpenMP offload support
target_compile_options(XSBench OFFLOAD)
target_link_libraries(XSBench OPENMP)

# Offload target for XSBench kernel
set_target_properties(XSBench PROPERTIES OFFLOAD ON)