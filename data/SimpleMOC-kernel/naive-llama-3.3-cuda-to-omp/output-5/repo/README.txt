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

SimpleMOC-kernel is an OpenMP-offload code and supports multiple architectures,
including x86, PowerPC, and NVIDIA GPUs.

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

	>$ make

	Note: You may need to set the compiler and flags according to your system.
	For example, you can add the following lines to your Makefile:
	```makefile
CC = gcc
CFLAGS = -fopenmp -foffload-nvptx=-std=c++14