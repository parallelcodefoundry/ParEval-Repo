// Offload Directive: target devices (GPU)
#pragma offload_attribute(push)
#pragma offload_attribute(target(same_device))

// Define the function that will be executed on the GPU
void initializeDeviceSources(
    Input I,
    Source_Arrays *SA_h,
    Source_Arrays *SA_d,
    Source *sources_h
);

// End of Offload Directive
#pragma offload_attribute(pop)

// Function to initialize device sources
void initializeDeviceSources(
    Input I,
    Source_Arrays *SA_h,
    Source_Arrays *SA_d,
    Source *sources_h
) {
  // Allocate & Copy Fine Source Data
  long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
  #pragma offload target(mgpu) in(SA_h->fine_source_arr[0:N_fine]) \
     out(SA_d->fine_source_arr[0:N_fine]) async(noreorder)
  cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);
  
  // Allocate & Copy Fine Flux Data
  #pragma offload target(mgpu) in(SA_h->fine_flux_arr[0:N_fine]) \
     out(SA_d->fine_flux_arr[0:N_fine]) async(noreorder)
  cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);
  
  // Allocate & Copy SigT Data
  long N_sigT = I.source_3D_regions * I.egroups;
  #pragma offload target(mgpu) in(SA_h->sigT_arr[0:N_sigT]) \
     out(SA_d->sigT_arr[0:N_sigT]) async(noreorder)
  cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);
  
  // Allocate & Copy Source Array Data
  #pragma offload target(mgpu) in(sources_h[0:I.source_3D_regions]) \
     out(sources_d[0:I.source_3D_regions]) async(noreorder)
  cudaMemcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);
}