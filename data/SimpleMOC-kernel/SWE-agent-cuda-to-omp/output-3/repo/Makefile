
CXX = g++
CXXFLAGS = -fopenmp -O3

# Source files
SRCS = main.cpp init.hpp kernel.hpp

# Output executable
TARGET = SimpleMOC

all: $(TARGET)

$(TARGET): $(SRCS)
$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
rm -f $(TARGET)