#pragma offload_attribute(push)
#pragma offload_attribute(init)
__global__ void kernel() {
    // kernel code goes here
}

void __cudaCheckError( const char *file, const int line ) {
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

#pragma offload_attribute(pop)

int main() {
    // kernel launch
    kernel<<<256, 256>>>();

    // flush any pending writes to the device
    __cudaCheckError(__FILE__, __LINE__);
}