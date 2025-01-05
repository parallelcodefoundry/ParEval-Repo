#include "XSbench_header.hpp"

// num_nucs represents the number of nuclides that each material contains
Kokkos::View<int*, Kokkos::HostSpace> load_num_nucs(long n_isotopes) {
    Kokkos::View<int*, Kokkos::HostSpace> num_nucs("num_nucs", 12);

    // Material 0 is a special case (fuel). The H-M small reactor uses
    // 34 nuclides, while H-M larges uses 300.
    if (n_isotopes == 68)
        num_nucs(0) = 34; // HM Small is 34, H-M Large is 321
    else
        num_nucs(0) = 321; // HM Small is 34, H-M Large is 321

    num_nucs(1) = 5;
    num_nucs(2) = 4;
    num_nucs(3) = 4;
    num_nucs(4) = 27;
    num_nucs(5) = 21;
    num_nucs(6) = 21;
    num_nucs(7) = 21;
    num_nucs(8) = 21;
    num_nucs(9) = 21;
    num_nucs(10) = 9;
    num_nucs(11) = 9;

    return num_nucs;
}

// Assigns an array of nuclide ID's to each material
Kokkos::View<int*, Kokkos::HostSpace> load_mats(const Kokkos::View<int*, Kokkos::HostSpace>& num_nucs, long n_isotopes, int* max_num_nucs) {
    *max_num_nucs = 0;
    int num_mats = 12;
    for (int m = 0; m < num_mats; m++) {
        if (num_nucs(m) > *max_num_nucs)
            *max_num_nucs = num_nucs(m);
    }
    Kokkos::View<int*, Kokkos::HostSpace> mats("mats", num_mats * (*max_num_nucs));

    // Small H-M has 34 fuel nuclides
    int mats0_Sml[] = {58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                       8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
                       54, 55, 56, 18, 23, 41}; //fuel
    // Large H-M has 300 fuel nuclides
    int mats0_Lrg[321] = {58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                         8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
                         54, 55, 56, 18, 23, 41}; //fuel
    for (int i = 0; i < 321 - 34; i++)
        mats0_Lrg[34 + i] = 68 + i; // H-M large adds nuclides to fuel only

    // These are the non-fuel materials
    int mats1[] = {63, 64, 65, 66, 67}; // cladding
    int mats2[] = {24, 41, 4, 5}; // cold borated water
    int mats3[] = {24, 41, 4, 5}; // hot borated water
    int mats4[] = {19, 20, 21, 22, 35, 36, 37, 38, 39, 25, 27, 28, 29,
                   30, 31, 32, 26, 49, 50, 51, 11, 12, 13, 14, 6, 16,
                   17}; // RPV
    int mats5[] = {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                   49, 50, 51, 11, 12, 13, 14}; // lower radial reflector
    int mats6[] = {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                   49, 50, 51, 11, 12, 13, 14}; // top reflector / plate
    int mats7[] = {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                   49, 50, 51, 11, 12, 13, 14}; // bottom plate
    int mats8[] = {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                   49, 50, 51, 11, 12, 13, 14}; // bottom nozzle
    int mats9[] = {24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                   49, 50, 51, 11, 12, 13, 14}; // top nozzle
    int mats10[] = {24, 41, 4, 5, 63, 64, 65, 66, 67}; // top of FA's
    int mats11[] = {24, 41, 4, 5, 63, 64, 65, 66, 67}; // bottom FA's

    // H-M large v small dependency
    if (n_isotopes == 68)
        Kokkos::deep_copy(mats, Kokkos::View<const int*, Kokkos::HostSpace>(mats0_Sml, num_nucs(0)));
    else
        Kokkos::deep_copy(mats, Kokkos::View<const int*, Kokkos::HostSpace>(mats0_Lrg, num_nucs(0)));

    // Copy other materials
    Kokkos::deep_copy(Kokkos::subview(mats, 1, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats1, num_nucs(1)));
    Kokkos::deep_copy(Kokkos::subview(mats, 2, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats2, num_nucs(2)));
    Kokkos::deep_copy(Kokkos::subview(mats, 3, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats3, num_nucs(3)));
    Kokkos::deep_copy(Kokkos::subview(mats, 4, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats4, num_nucs(4)));
    Kokkos::deep_copy(Kokkos::subview(mats, 5, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats5, num_nucs(5)));
    Kokkos::deep_copy(Kokkos::subview(mats, 6, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats6, num_nucs(6)));
    Kokkos::deep_copy(Kokkos::subview(mats, 7, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats7, num_nucs(7)));
    Kokkos::deep_copy(Kokkos::subview(mats, 8, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats8, num_nucs(8)));
    Kokkos::deep_copy(Kokkos::subview(mats, 9, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats9, num_nucs(9)));
    Kokkos::deep_copy(Kokkos::subview(mats, 10, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats10, num_nucs(10)));
    Kokkos::deep_copy(Kokkos::subview(mats, 11, Kokkos::ALL()), Kokkos::View<const int*, Kokkos::HostSpace>(mats11, num_nucs(11)));

    return mats;
}

// Randomizes the concentrations of all nuclides in a variety of materials
Kokkos::View<double*, Kokkos::HostSpace> load_concs(const Kokkos::View<int*, Kokkos::HostSpace>& num_nucs, int max_num_nucs) {
    uint64_t seed = STARTING_SEED * STARTING_SEED;
    Kokkos::View<double*, Kokkos::HostSpace> concs("concs", 12 * max_num_nucs);

    for (int i = 0; i < 12; i++)
        for (int j = 0; j < num_nucs(i); j++)
            concs(i * max_num_nucs + j) = LCG_random_double(&seed);

    return concs;
}