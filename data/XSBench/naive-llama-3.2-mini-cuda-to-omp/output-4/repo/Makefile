# Makefile for XSBench

# Compilation flags
CFLAGS = -Wall -O3 -fopenmp -fPIC
ifeq ($(SM_VERSION),80)
    CFLAGS += -D __GLES2__
endif

# Shared library flags
SHARED_FLAGS = -shared -fPIC

# Object files
OBJS = XSBench.o Utilities.o Profile.o
ifeq ($(SM_VERSION),80)
    OBJS += GPUUtilities.o GPUProfile.o
endif

# Header files
HINC = XSBench.h Utilities.h Profile.h
ifeq ($(SM_VERSION),80)
    HINC += GPUUtilities.h GPUProfile.h
endif

# Source files
SRC = $(OBJS:.o=.cpp)

# Build rules
$(foreach obj,$(OBJS),$(obj:.o=.o))
$(foreach obj,$(OBJS:.o=.cpp),$($(obj):=$(obj).o))
$(foreach obj,$(OBJS:.o=.cpp),$($(obj):=$(obj).o))

%.o: %.cpp
	@echo "Compiling $<"
	omp -fopenmp $(CFLAGS) -c $< -o $@

# Link rules
XSBench: $(OBJS)
	@echo "Linking XSBench"
	$(CC) $(SHARED_FLAGS) $(HINC) -o $@ $^

Utilities: Utilities.o
	@echo "Linking Utilities"
	$(CC) $(SHAREDFlags) $(HINC) -o $@ $^

Profile: GPUProfile.o
	@echo "Linking Profile"
	$(CC) $(SHARED_flags) $(HINC) -o $@ $^

ifneq ($(SM_VERSION),80)
%.o: %.cpp
	omp -fopenmp $(CFLAGS) -c $< -o $@
endif

# Dependencies
XSBench: Utilities Profile
Utilities: 
Profile: 

# Clean rule
clean:
    @echo "Cleaning..."
    rm -f *.o XSBench Utilities Profile GPUUtilities GPUProfile

.PHONY: clean