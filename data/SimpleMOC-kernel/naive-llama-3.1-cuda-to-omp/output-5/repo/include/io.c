#include <omp.h>

// ... (rest of the file remains the same)

void __cudaCheckError( const char *file, const int line )
{
#ifdef CUDA_ERROR_CHECK
    cudaError err = cudaGetLastError();
    if ( cudaSuccess != err )
    {
        fprintf( stderr, "cudaCheckError() failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString( err ) );
        exit( -1 );
    }
 
    // More careful checking. However, this will affect performance.
    // Comment away if needed.
    err = cudaDeviceSynchronize();
    if( cudaSuccess != err )
    {
        fprintf( stderr, "cudaCheckError() with sync failed at %s:%i : %s\n",
                 file, line, cudaGetErrorString( err ) );
        exit( -1 );
    }
#endif
 
    return;
}

// ... (rest of the file remains the same)

int main() {
  // Initialize OpenMP environment
  int num_devices = omp_get_num_devices();
  #pragma omp target teams distribute parallel for device_range(num_devices) default(none)
  for(int i=0; i<num_devices;i++){
    std::cout << "Device " << i << std::endl;
  }
  
  // ... (rest of the main function remains the same)

  // Initialize CUDA environment
  cudaError_t err = cudaInit();
  if( cudaSuccess != err ) {
      fprintf(stderr, "cudaInit() failed : %s\n", cudaGetErrorString(err));
      exit(-1);
  }
  
  // ... (rest of the file remains the same)
}