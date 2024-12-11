#include "XSbench_header.cuh"

// Material data is hard coded into the functions in this file.
// Note that there are 12 materials present in H-M (large or small)

#define MAX_NUM_NUCS 321

int * load_num_nucs(long n_isotopes)
{
    int num_mats = 12;
    int max_num_nucs[num_mats];
    int *num_nucs = (int*)malloc(num_mats*sizeof(int));
    for (int m = 0; m < num_mats; m++) {
        if (m == 0) { // Special case: fuel
            if (n_isotopes == 68)
                num_nucs[0] = 34;
            else
                num_nucs[0] = 321;
        } else {
            num_nucs[m] = MAX_NUM_NUCS; // Default max nuclides per material
        }
    }

    return num_nucs;
}

// Assigns an array of nuclide ID's to each material
int * load_mats(int *num_nucs, long n_isotopes, int *max_num_nucs)
{
    int num_mats = 12;

#pragma omp declare simd
#define MATS0_SML {58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7, \
                   8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53, \
                   54, 55, 56, 18, 23, 41} // fuel

#pragma omp declare simd
#define MATS0_LRG {58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7, \
                   8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53, \
                   54, 55, 56, 18, 23, 41} // fuel

#pragma omp declare simd
#define MATS1 {63, 64, 65, 66, 67} // cladding
#pragma omp declare simd
#define MATS2 {24, 41, 4, 5} // cold borated water
#pragma omp declare simd
#define MATS3 {24, 41, 4, 5} // hot borated water
#pragma omp declare simd
#define MATS4 {19, 20, 21, 22, 35, 36, 37, 38, 39, 25, 27, 28, 29, \
               30, 31, 32, 26, 49, 50, 51, 11, 12, 13, 14, 6, 16, \
               17} // RPV
#pragma omp declare simd
#define MATS5 {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, \
               49, 50, 51, 11, 12, 13, 14} // lower radial reflector
#pragma omp declare simd
#define MATS6 {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, \
               49, 50, 51, 11, 12, 13, 14} // top reflector / plate
#pragma omp declare simd
#define MATS7 {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, \
               49, 50, 51, 11, 12, 13, 14} // bottom plate
#pragma omp declare simd
#define MATS8 {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, \
               49, 50, 51, 11, 12, 13, 14} // bottom nozzle
#pragma omp declare simd
#define MATS9 {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, \
               49, 50, 51, 11, 12, 13, 14} // top nozzle
#pragma omp declare simd
#define MATS10 {24, 41, 4, 5, 63, 64, 65, 66, 67} // top of FA's
#pragma omp declare simd
#define MATS11 {24, 41, 4, 5, 63, 64, 65, 66, 67} // bottom FA's

    int *mats = (int*)malloc(num_mats*MAX_NUM_NUCS*sizeof(int));

    if (n_isotopes == 68) {
        memcpy(mats, MATS0_SML, num_nucs[0] * sizeof(int));
    } else {
        memcpy(mats, MATS0_LRG, num_nucs[0] * sizeof(int));
    }

#pragma omp parallel for simd
    for (int m = 1; m < num_mats; m++) {
        int mat_size;
        if (m == 1) { // cladding
            mat_size = sizeof(MATS1)/sizeof(MATS1[0]);
        } else if (m == 2 || m == 3) { // borated water
            mat_size = sizeof(MATS2)/sizeof(MATS2[0]);
        } else if (m == 4) { // RPV
            mat_size = sizeof(MATS4)/sizeof(MATS4[0]);
        } else if (m == 5) { // lower radial reflector
            mat_size = sizeof(MATS5)/sizeof(MATS5[0]);
        } else if (m == 6) { // top reflector / plate
            mat_size = sizeof(MATS6)/sizeof(MATS6[0]);
        } else if (m == 7) { // bottom plate
            mat_size = sizeof(MATS7)/sizeof(MATS7[0]);
        } else if (m == 8) { // bottom nozzle
            mat_size = sizeof(MATS8)/sizeof(MATS8[0]);
        } else if (m == 9) { // top nozzle
            mat_size = sizeof(MATS9)/sizeof(MATS9[0]);
        } else if (m == 10 || m == 11) { // FA's
            mat_size = sizeof(MATS10)/sizeof(MATS10[0]);
        }
        memcpy(mats + MAX_NUM_NUCS * m, mats1 + m - 1, mat_size * sizeof(int));
    }

    return mats;
}

// Randomizes the concentrations of all nuclides in a variety of materials
double * load_concs(int *num_nucs, int max_num_nucs)
{
    uint64_t seed = STARTING_SEED * STARTING_SEED;

#pragma omp declare simd
#define CONCS {LCG_random_double(&seed), LCG_random_double(&seed), \
               LCG_random_double(&seed), LCG_random_double(&seed)}

#pragma omp parallel for simd
    double *concs = (double*)malloc(num_nucs[0] * max_num_nucs * sizeof(double));
    for (int i = 0; i < num_nucs[0]; i++) {
        concs[i*max_num_nucs + 0] = CONCS[0];
        concs[i*max_num_nucs + 1] = CONCS[1];
        concs[i*max_num_nucs + 2] = CONCS[2];
        concs[i*max_num_nucs + 3] = CONCS[3];
    }
    // test
    /*
    for (int i = 0; i < num_nucs[0]; i++)
        for (int j = 0; j < max_num_nucs; j++) {
            printf("concs[%d][%d] = %lf\n", i, j, concs[i*max_num_nucs + j]);
        }
    */

    return concs;
}