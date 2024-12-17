===============================================================================
    
              _____ _                 _      __  __  ____   _____ 
             / ____(_)               | |    |  \/  |/ __ \ / ____|
            | (___  _ _ __ ___  _ __ | | ___| \  / | |  | | |     
             \___ \| | '_ ` _ \| '_ \| |/ _ \ |\/| | |  | | |     
             ____) | | | | | | | |_) | |  __/ |  | | |__| | |____ 
            |_____/|_|_| |_| |_| .__/|_|\___|_|  |_|\____/ \_____|
                               | |                                
                               |_|                                
                           _  __                    _ 
                          | |/ /___ _ __ _ __   ___| |
                          | ' // _ \ '__| '_ \ / _ \ |
                          | . \  __/ |  | | | |  __/ |
                          |_|\_\___|_|  |_| |_|\___|_|


                                   Version 4

==============================================================================
Contact Information
==============================================================================

Organizations:     Computational Reactor Physics Group
                   Massachusetts Institute of Technology

                   Center for Exascale Simulation of Advanced Reactors (CESAR)
                   Argonne National Laboratory

Development Leads: John Tramm     <jtramm@mit.edu>
                   Geoffrey Gunow <geogunow@mit.edu>
                   Tim He         <shuohe@anl.gov>
                   Ron Rahaman    <rahaman@anl.gov>
                   Amanda Lund    <alund@anl.gov>
    
===============================================================================
What is SimpleMOC-kernel?
===============================================================================

SimpleMOC-kernel represents the core computational of a larger application
(SimpleMOC). This app was written in order to abstract away much of the
complexity of the full application in order to facilitate easier porting of
the code and enable more transparent analysis techniques on high performance
architectures.

The scope of this kernel is essentially the inner-loop of SimpleMOC, i.e., the
attentuation of neutron fluxes across an individual geometrical segment.
This kernel composes approximately 92% of the walltime of the full application,
and is therefore useful for analyzing optimization methods and performance
implications for exascale supercomputer architectures.

More information can be found in the following publication:

http://dx.doi.org/10.1016/j.cpc.2016.01.007

==============================================================================
Architectural Support
==============================================================================

SimpleMOC-kernel is an OpenMP code that supports the NVIDIA Tesla GPU architecture,
offloaded to the device using the OpenMP offload directive.

==============================================================================
Quick Start Guide
==============================================================================

Download----------------------------------------------------------------------

	For the most up-to-date version of SimpleMOC-kernel, we recommend that you
	download from our git repository. This can be accomplished via
	cloning the repository from the command line, or by downloading a zip
	from our github page.

	Git Repository Clone:
		
		Use the following command to clone SimpleMOC-kernel to your machine:

		>$ git clone https://github.com/ANL-CESAR/SimpleMOC-kernel.git

		Once cloned, you can update the code to the newest version
		using the following command (when in the SimpleMOC-kernel directory):

		>$ git pull

Compilation-------------------------------------------------------------------

	To compile SimpleMOC-kernel with default settings, use the following command:

	>$ make -f Makefile.c

Running SimpleMOC-kernel-------------------------------------------------------

	To run SimpleMOC-kernel with default settings, use the following command:

	>$ ./SimpleMOC-kernel -nthreads 1 -s 50000000 -e 128 -p 100

	For non-default settings, SimpleMOC-kernel supports the following
	command line options:

	Usage: ./SimpleMOC-kernel <options>
	Options include:
	  -t <threads>          Number of threads to run
	  -s <segments>         Number of segments to process
	  -e <energy groups>    Number of energy groups
	  -p <segs per thread>  Number of segments per OpenMP block
	

	If not options are specified, then a default set of parameters will
	automatically be run. These parameters reflect the approximate per node
	work load for a full core reactor simulation (the the number of geometry
	segments has been signficantly reduced to reduce runtime while preserving
	the computational profile).

==============================================================================
Advanced Compilation, Debugging, Optimization, and Profiling
==============================================================================

There are a number of switches that can be set at the top of the Makefile.c,
along with more advanced compilation features.

Here is a sample of the control panel at the top of the Makefile:

COMPILER    = clang++ -fopenmp-targets=nvptx64-nvidia-cuda
OPTIMIZE    = yes
DEBUG       = no
PROFILE     = no

Explanation of Flags:

COMPILER <clang++> - This selects your compiler (NVIDIA is only one supported).

OPTIMIZE - Adds compiler optimization flag "-O3" and other optimizations.

DEBUG - Adds the compiler flag "-g".

PROFILE - Adds the compiler flag "-pg".