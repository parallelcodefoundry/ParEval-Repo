#include "XSbench_header.h"

int * load_num_nucs(long n_isotopes)
{
    int * num_nucs = (int*)malloc(12*sizeof(int));

    // Material 0 is a special case (fuel). The H-M small reactor uses
    // 34 nuclides, while H-M larges uses 300.
    if( n_isotopes == 68 )
        num_nucs[0]  = 34; // HM Small is 34, H-M Large is 321
    else
        num_nucs[0]  = 321; // HM Small is 34, H-M Large is 321

    #pragma offload target(mic:arch=skx) in(num_nucs[12]) out(num_nucs)
    {
        #pragma omp parallel for
        for(int i = 1; i < 12; i++)
            num_nucs[i]  = load_num_nucs_helper(i, n_isotopes);
    }

    return num_nucs;
}

int load_num_nucs_helper(int material, long n_isotopes)
{
    int nuclides;
    if (material == 0) {
        // Small H-M has 34 fuel nuclides
        if(n_isotopes == 68){
            nuclides = 34; //fuel
        }
        else {
            nuclides = 321; //fuel
        }
    } else if(material == 1) {
        nuclides = 5;
    } else if(material == 2) {
        nuclides = 4;
    } else if(material == 3) {
        nuclides = 4;
    } else if(material == 4) {
        nuclides = 27;
    } else if(material == 5) {
        nuclides = 21;
    } else if(material == 6) {
        nuclides = 21;
    } else if(material == 7) {
        nuclides = 21;
    } else if(material == 8) {
        nuclides = 21;
    } else if(material == 9) {
        nuclides = 21;
    } else if(material == 10) {
        nuclides = 9;
    } else if(material == 11) {
        nuclides = 9;
    }

    return nuclides;
}

int * load_mats( int * num_nucs, long n_isotopes, int * max_num_nucs )
{
    #pragma offload target(mic:arch=skx) in(num_nucs[12]) out(num_nucs)
    {
        *max_num_nucs = 0;
        for (int m = 0; m < 12; m++)
        {
            if( num_nucs[m] > *max_num_nucs )
                *max_num_nucs = num_nucs[m];
        }

        #pragma omp parallel for
        for(int i = 0; i < 12; i++) {
            int nuclides;
            if (i == 0) {
                // Small H-M has 34 fuel nuclides
                if(n_isotopes == 68){
                    nuclides = 34; //fuel
                } else {
                    nuclides = 321; //fuel
                }
            } else if(i == 1) {
                nuclides = 5;
            } else if(i == 2) {
                nuclides = 4;
            } else if(i == 3) {
                nuclides = 4;
            } else if(i == 4) {
                nuclides = 27;
            } else if(i == 5) {
                nuclides = 21;
            } else if(i == 6) {
                nuclides = 21;
            } else if(i == 7) {
                nuclides = 21;
            } else if(i == 8) {
                nuclides = 21;
            } else if(i == 9) {
                nuclides = 21;
            } else if(i == 10) {
                nuclides = 9;
            } else if(i == 11) {
                nuclides = 9;
            }

            int mats_i[(*max_num_nucs)] = {0};
            if (i == 0) {
                // Small H-M has 34 fuel nuclides
                if(n_isotopes == 68){
                    memcpy(mats_i, mats0_Sml, num_nucs[i] * sizeof(int));
                } else {
                    memcpy(mats_i, mats0_Lrg, num_nucs[i] * sizeof(int));
                }
            } else {
                // Copy other materials
                if (i == 1) {
                    memcpy(mats_i, mats1, num_nucs[i] * sizeof(int));
                } else if(i == 2) {
                    memcpy(mats_i, mats2, num_nucs[i] * sizeof(int));
                } else if(i == 3) {
                    memcpy(mats_i, mats3, num_nucs[i] * sizeof(int));
                } else if(i == 4) {
                    memcpy(mats_i, mats4, num_nucs[i] * sizeof(int));
                } else if(i == 5) {
                    memcpy(mats_i, mats5, num_nucs[i] * sizeof(int));
                } else if(i == 6) {
                    memcpy(mats_i, mats6, num_nucs[i] * sizeof(int));
                } else if(i == 7) {
                    memcpy(mats_i, mats7, num_nucs[i] * sizeof(int));
                } else if(i == 8) {
                    memcpy(mats_i, mats8, num_nucs[i] * sizeof(int));
                } else if(i == 9) {
                    memcpy(mats_i, mats9, num_nucs[i] * sizeof(int));
                } else if(i == 10) {
                    memcpy(mats_i, mats10, num_nucs[i] * sizeof(int));
                } else if(i == 11) {
                    memcpy(mats_i, mats11, num_nucs[i] * sizeof(int));
                }
            }

            #pragma offload target(mic:arch=skx) in(num_nucs[12]) out(num_nucs)
            {
                for (int j = 0; j < nuclides; j++)
                    num_nucs[i + j] = mats_i[j];
            }
        }
    }

    return num_nucs;
}