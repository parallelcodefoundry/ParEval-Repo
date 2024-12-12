#include "XSbench_header.h"

////////////////////////////////////////////////////////////////////////////////////
// BASELINE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////
// All "baseline" code is at the top of this file. The baseline code is a simple
// port of the original CPU OpenMP code to OpenMP offloading with few significant changes or
// optimizations made. Following these functions are a number of optimized variants,
// which each deploy a different combination of optimizations strategies. By
// default, XSBench will only run the baseline implementation. Optimized variants
// must be specifically selected using the "-k <optimized variant ID>" command
// line argument.
////////////////////////////////////////////////////////////////////////////////////

unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile)
{
	double start = get_time();

        // Move Data to Device
#pragma omp target data map(to: in, SD.num_nucs[0:SD.length_num_nucs], SD.concs[0:SD.length_concs], SD.mats[0:SD.length_mats], SD.unionized_energy_array[0:SD.length_unionized_energy_array], SD.index_grid[0:SD.length_index_grid], SD.nuclide_grid[0:SD.length_nuclide_grid]) map(alloc: SD.verification[0:SD.length_verification])
	{
        	profile->host_to_device_time = get_time() - start;
        	start = get_time();

        	////////////////////////////////////////////////////////////////////////////////
        	// Configure & Launch Simulation Kernel
        	////////////////////////////////////////////////////////////////////////////////
        	if( mype == 0)	printf("Running baseline event-based simulation...\n");

        	int nthreads = 256;
		// Adjust nthreads for OpenMP offloading
		nthreads = omp_get_max_threads();
        	int nblocks = ceil( (double) in.lookups / (double) nthreads);

		int nwarmups = in.num_warmups;
		start = 0.0;
		for (int i = 0; i < in.num_iterations + nwarmups; i++) {
			if (i == nwarmups) {
				start = get_time();
			}
#pragma omp target teams distribute parallel for num_threads(nthreads)
			for (long i = 0; i < in.lookups; i++) {
				xs_lookup_kernel_baseline(in, SD, i);
			}
		}
		profile->kernel_time = get_time() - start;

		////////////////////////////////////////////////////////////////////////////////
		// Reduce Verification Results
		////////////////////////////////////////////////////////////////////////////////

        	if( mype == 0)	printf("Reducing verification results...\n");
		start = get_time();
#pragma omp target update from(SD.verification[0:SD.length_verification])
		profile->device_to_host_time = get_time() - start;
	}

        unsigned long verification_scalar = 0;
        for( int i =0; i < in.lookups; i++ )
                verification_scalar += SD.verification[i];


        return verification_scalar;
}


void xs_lookup_kernel_baseline(Inputs in, SimulationData SD, long i)
{
        // The lookup ID. Used to set the seed, and to store the verification value

        // Set the initial seed value
        uint64_t seed = STARTING_SEED;

        // Forward seed to lookup index (we need 2 samples per lookup)
        seed = fast_forward_LCG(seed, 2*i);

        // Randomly pick an energy and material for the particle
        double p_energy = LCG_random_double(&seed);
        int mat         = pick_mat(&seed);

        double macro_xs_vector[5] = {0};

        // Perform macroscopic Cross Section Lookup
        calculate_macro_xs(
                p_energy,        // Sampled neutron energy (in lethargy)
                mat,             // Sampled material type index neutron is in
                in.n_isotopes,   // Total number of isotopes in simulation
                in.n_gridpoints, // Number of gridpoints per isotope in simulation
                SD.num_nucs,     // 1-D array with number of nuclides per material
                SD.concs,        // Flattened 2-D array with concentration of each nuclide in each material
                SD.unionized_energy_array, // 1-D Unionized energy array
                SD.index_grid,   // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                SD.mats,         // Flattened 2-D array with nuclide indices defining composition of each type of material
                macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                in.grid_type,    // Lookup type (nuclide, hash, or unionized)
                in.hash_bins,    // Number of hash bins used (if using hash lookup type)
                SD.max_num_nucs  // Maximum number of nuclides present in any material
        );

        // For verification, and to prevent the compiler from optimizing
        // all work out, we interrogate the returned macro_xs_vector array
        // to find its maximum value index, then increment the verification
        // value by that index. In this implementation, we have each thread
        // write to its thread_id index in an array, which we will reduce
        // with a thrust reduction kernel after the main simulation kernel.
        double max = -1.0;
        int max_idx = 0;
        for(int j = 0; j < 5; j++ )
        {
                if( macro_xs_vector[j] > max )
                {
                        max = macro_xs_vector[j];
                        max_idx = j;
                }
        }
        SD.verification[i] = max_idx+1;
}

// Calculates the microscopic cross section for a given nuclide & energy
void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
                                   long n_gridpoints,
                                   double * __restrict__ egrid, int * __restrict__ index_data,
                                   NuclideGridPoint * __restrict__ nuclide_grids,
                                   long idx, double * __restrict__ xs_vector, int grid_type, int hash_bins )
{
	// ... (Implementation remains the same)
}

// Calculates macroscopic cross section based on a given material & energy
void calculate_macro_xs( double p_energy, int mat, long n_isotopes,
                                   long n_gridpoints, int * __restrict__ num_nucs,
                                   double * __restrict__ concs,
                                   double * __restrict__ egrid, int * __restrict__ index_data,
                                   NuclideGridPoint * __restrict__ nuclide_grids,
                                   int * __restrict__ mats,
                                   double * __restrict__ macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs )
{
	// ... (Implementation remains the same)
}


// binary search for energy on unionized energy grid
// returns lower index
long grid_search( long n, double quarry, double * __restrict__ A)
{
	// ... (Implementation remains the same)
}

// binary search for energy on nuclide energy grid
long grid_search_nuclide( long n, double quarry, NuclideGridPoint * A, long low, long high)
{
	// ... (Implementation remains the same)
}

// picks a material based on a probabilistic distribution
int pick_mat( uint64_t * seed )
{
	// ... (Implementation remains the same)
}

double LCG_random_double(uint64_t * seed)
{
	// ... (Implementation remains the same)
}

uint64_t fast_forward_LCG(uint64_t seed, uint64_t n)
{
	// ... (Implementation remains the same)
}

// ... (Optimized variant functions remain largely the same, requiring similar offloading strategies)
