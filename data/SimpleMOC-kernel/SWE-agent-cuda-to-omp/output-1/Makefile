
CXX = g++
CXXFLAGS = -fopenmp -O3 -Wall

# Source files
SOURCES = main.hpp init.hpp kernel.hpp io.hpp

# Output executable
OUTPUT = SimpleMOC

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(SOURCES)

clean:
rm -f $(OUTPUT)