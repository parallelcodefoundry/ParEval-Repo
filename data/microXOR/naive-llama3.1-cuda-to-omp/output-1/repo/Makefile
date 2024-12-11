# Use compiler flags to enable OpenMP and offloading
CXX = $(CXX_COMPILER)
CXXFLAGS = -fopenmp -march=$(SM_VERSION) -cuda

# Directories and files
SRCDIR = src
BUILDDIR = build
BINDIR = bin

# Source and header files
SOURCES = $(wildcard $(SRCDIR)/microXOR.cpp) $(wildcard $(SRCDIR)/microXOR.h)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES:.h=.cpp))

# Build target
TARGET = $(BINDIR)/microXOR

# Rules
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean up
clean:
	rm -rf $(BUILDDIR) $(BINDIR)/microXOR

# Target to build the executable for a specific SM version
build-%:
	$(MAKE) CXX_COMPILER=$(shell which clang++) SM_VERSION=$* TARGET=$(TARGET)

# Default target
default: build-80

.PHONY: clean default