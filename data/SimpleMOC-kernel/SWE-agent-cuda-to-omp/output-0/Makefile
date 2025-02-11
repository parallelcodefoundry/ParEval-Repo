
CXX = g++
CXXFLAGS = -fopenmp -O3

# Source files
SRCS = init.cpp kernel.cpp main.cpp

# Output executable
TARGET = SimpleMOC

all: $(TARGET)

$(TARGET): $(SRCS)
$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
rm -f $(TARGET)