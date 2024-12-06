# OpenMP-Offload Execution Model Makefile

CC = gcc -fopenmp-offload
CFLAGS = -O3 -std=c11 -D __STRICT_ANSI__ -Wall -Wextra -fno-pure-inclusions
LDFLAGS = -shared

# Input files
XSBENCH_SHARED_HEADER_H = xsbench_shared_header.h
XSBDATA = xsbdata.c
XSBEVENT = xsbevent.c
XSBSORT = xsbsort.c
XSBNUMERICS = xsbnumerics.c
XSBSIMULATION = xsbsimulation.c

# Dependencies
XSBENCH_SHARED_HEADER_HDep = xsbench_shared_header.h
XSBDATADep = xsbdata.c
XSBEVENTDep = xsbevent.c
XSBSORTDep = xsbsort.c
XSBNUMERICSDep = xsbnumerics.c
XSBSIMULATIONDep = xsbsimulation.c

# Output files
XSBENCH_SHARED_HEADER_H_out = xsbench_shared_header.h.o
XSBDATA_out = xsbdata.o
XSBEVENT_out = xsbevent.o
XSBSORT_out = xsbsort.o
XSBNUMERICS_out = xsbnumerics.o
XSBSIMULATION_out = xsbsimulation.o

# Flags for OpenMP-offload execution model
OMPFALLBACK = -fopenmp-fallback
OOPENMP = -fopenmp-offload

# CFLAGS and LDFLAGS for openmp-offload code
CFLAGS_openmp_offload = $(CFLAGS) $(OMPFALLBACK)
LDFLAGS_openmp_offload = $(LDFLAGS)

# Dependencies for OpenMP-offload executable
XSBENCH_SHARED_HEADER_HDep_o = xsbench_shared_header.h.o.o
XSBDATADep_o = xsbdata.o.o
XSBEVENTDep_o = xsbevent.o.o
XSBSORTDep_o = xsbsort.o.o
XSBNUMERICSDep_o = xsbnumerics.o.o
XSBSIMULATIONDep_o = xsbsimulation.o.o

# Linking flags for OpenMP-offload code
LDFLAGS_openmp_offload = $(LDFLAGS) $(OOPENMP)

# Makefile rules
%.o: %.c $(OMPFALLBACK)
    $(CC) -c $< -o $@ -I$(OMPFALLBACK)

%.o.o: %.c $(CFLAGS_openmp_offload)
    $(CC) -c $< -o $@ -I$(CFLAGS_openmp_offload) $(LDFLAGS_openmp_offload)

# All objects
all_objects = $(XSBDATA_out) $(XSBEVENT_out) $(XSBSORT_out) $(XSBNUMERICS_out) $(XSBSIMULATION_out)

# All linked objects
all_linked_objects = $(XSBENCH_SHARED_HEADER_H_out.o) $(XSBDATA_out.o) $(XSBEVENT_out.o) $(XSBSORT_out.o) $(XSBNUMERICS_out.o) $(XSBSIMULATION_out.o)

$(XSBSIMULATION): all_linked_objects

# Make rule for linked object
%.o: %.c $(CFLAGS_openmp_offload)
    $(CC) -c $< -o $@ -I$(CFLAGS_openmp_offload) $(LDFLAGS_openmp_offload)

$(XSBSIMULATION): all_linked_objects
    $(CC) $(OOPENMP) -shared -o $@ $^ $(LDFLAGS_openmp_offload)