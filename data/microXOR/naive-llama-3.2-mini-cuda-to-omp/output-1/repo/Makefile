# Makefile for OpenMP-offload microXOR

# Define flags and compiler options
CXX=g++
CXXFLAGS=-std=c++11 -O2
LDFLAGS=-shared -fopenmp
# For OpenMP-offload, we use the --target option to specify the target architecture.
# Since the problem mentions an NVIDIA GPU with compute capability 80, we assume that the system has a CUDA-enabled architecture.
CXXFLAGS+=--target=cuda-arch=sm_80

# Define filenames and paths
SM_VERSION=80
NAME=microXOR_offload
SRCDIR=$(pwd)
BUILDDIR=$(basename $(dirname $(pwd)))/${NAME}_${SM_VERSION}
BUILD_DIR=${BUILDDIR}/build
INSTALL_DIR=${BUILDDIR}/install

# Create build and install directories if they do not exist
mkdir -p ${BUILD_DIR} ${INSTALL_DIR}

# Copy source files to the build directory
cp ${SRCDIR}/${NAME}.cpp ${BUILDDIR}/src/

# Compile source code using OpenMP-offload compiler flags
$(CXX) $(CXXFLAGS) -c ${BUILDDIR}/src/microXOR.cpp -o ${BUILD_DIR}/microXOR.o

# Create a dynamic library to link with OpenMP runtime
$(CXX) $(LDFLAGS) -shared -fPIC -o ${BUILD_DIR}/lib${NAME}.so ${BUILD_DIR}/microXOR.o -lcuda

# Link the main program and create an executable
$(CXX) $(CXXFLAGS) -c ${SRCDIR}/main.cpp -o ${BUILDDIR}/src/main.o
$(CXX) $(LDFLAGS) -shared -fPIC -o ${INSTALL_DIR}/${NAME}_${SM_VERSION} ${BUILD_DIR}/lib${NAME}.so ${Builddir}/src/main.o

# Install the executable to a directory
install -d ${INSTALL_DIR}
mv ${INSTALL_DIR}/${NAME}_${SM_VERSION} ${INSTALL_DIR}/bin/

# Remove build files if not explicitly requested
if [ "$$" -ne 0 ]; then
  rm -rf ${BUILDDIR}
fi