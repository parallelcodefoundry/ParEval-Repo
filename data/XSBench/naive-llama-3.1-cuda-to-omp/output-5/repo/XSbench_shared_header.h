#ifndef XSBENCH_TRANSLATED_HEADER_H
#define XSBENCH_TRANSLATED_HEADER_H

// Header for translated utilities across XSBench versions

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct{
        int nthreads;
        long n_isotopes;
        long n_gridpoints;
        int lookups;
        char * HM;
        int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
        int hash_bins;
        int particles;
        int simulation_method;
        int binary_mode;
        int kernel_id;
        int num_iterations;
        int num_warmups;
        char *filename;
} Inputs;

#ifdef __cplusplus
}
#endif // __cplusplus

// Macro to handle OpenMP offload
#define XSBENCH_OFFLOAD( code ) \
  do { \
    #ifdef _OPENMP \
      if (omp_get_num_devices() > 0) { \
        #pragma omp target teams distribute parallel for simd collapse(2) num_teams(in.nthreads, in.particles) \
          map(to: from(in.HM[0])) \
          map(tofrom: from(in.HM[in.hash_bins]) to(out.HM)) \
          if(omp_get_team_num() == 0) code; \
        #pragma omp target teams distribute parallel for simd collapse(2) num_teams(in.nthreads, in.particles) \
          map(to: from(in.HM[0])) \
          map(tofrom: from(in.HM[in.hash_bins]) to(out.HM)) \
          if(omp_get_team_num() == 1) code; \
      } else { \
        code; \
      } \
    #else // _OPENMP \
      code; \
    #endif // _OPENMP

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

inline void print_profile(Profile profile, Inputs in) {
  if (in.filename) {
    FILE* output = fopen(in.filename, "w");
    fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    fprintf(output, "%f,%f,%f,%d,%d\n",
            profile.host_to_device_time*1000,
            profile.kernel_time*1000,
            profile.device_to_host_time*1000,
            in.num_iterations,
            in.num_warmups);
    fclose(output);
  }
  else {
    printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    printf("%f,%f,%f,%d,%d\n",
           profile.host_to_device_time*1000,
           profile.kernel_time*1000,
           profile.device_to_host_time*1000,
           in.num_iterations,
           in.num_warmups);
  }
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // XSBENCH_TRANSLATED_HEADER_H