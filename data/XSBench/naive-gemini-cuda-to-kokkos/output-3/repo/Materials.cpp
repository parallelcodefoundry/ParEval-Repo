#include "XSbench_header.hpp"

// num_nucs represents the number of nuclides that each material contains
std::vector<int> load_num_nucs(long n_isotopes) {
  std::vector<int> num_nucs(12);

  // Material 0 is a special case (fuel). The H-M small reactor uses
  // 34 nuclides, while H-M larges uses 300.
  if (n_isotopes == 68)
    num_nucs[0] = 34; // HM Small is 34, H-M Large is 321
  else
    num_nucs[0] = 321; // HM Small is 34, H-M Large is 321

  num_nucs[1] = 5;
  num_nucs[2] = 4;
  num_nucs[3] = 4;
  num_nucs[4] = 27;
  num_nucs[5] = 21;
  num_nucs[6] = 21;
  num_nucs[7] = 21;
  num_nucs[8] = 21;
  num_nucs[9] = 21;
  num_nucs[10] = 9;
  num_nucs[11] = 9;

  return num_nucs;
}

// Assigns an array of nuclide ID's to each material
std::vector<int> load_mats(const std::vector<int>& num_nucs, long n_isotopes, int* max_num_nucs) {
  *max_num_nucs = 0;
  int num_mats = 12;
  for (int m = 0; m < num_mats; m++) {
    if (num_nucs[m] > *max_num_nucs)
      *max_num_nucs = num_nucs[m];
  }
  std::vector<int> mats(num_mats * (*max_num_nucs));

  // Small H-M has 34 fuel nuclides
  int mats0_Sml[] = {58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                     8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
                     54, 55, 56, 18, 23, 41}; //fuel
  // Large H-M has 300 fuel nuclides
  std::vector<int> mats0_Lrg(321);
  std::copy(std::begin(mats0_Sml), std::end(mats0_Sml), std::begin(mats0_Lrg));
  for (int i = 0; i < 321 - 34; i++)
    mats0_Lrg[34 + i] = 68 + i; // H-M large adds nuclides to fuel only

  // These are the non-fuel materials
  int mats1[] = {63, 64, 65, 66, 67}; // cladding
  int mats2[] = {24, 41, 4, 5};     // cold borated water
  int mats3[] = {24, 41, 4, 5};     // hot borated water
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
    std::copy(std::begin(mats0_Sml), std::end(mats0_Sml), std::begin(mats));
  else
    std::copy(std::begin(mats0_Lrg), std::end(mats0_Lrg), std::begin(mats));

  // Copy other materials
  std::copy(std::begin(mats1), std::end(mats1), std::begin(mats) + (*max_num_nucs) * 1);
  std::copy(std::begin(mats2), std::end(mats2), std::begin(mats) + (*max_num_nucs) * 2);
  std::copy(std::begin(mats3), std::end(mats3), std::begin(mats) + (*max_num_nucs) * 3);
  std::copy(std::begin(mats4), std::end(mats4), std::begin(mats) + (*max_num_nucs) * 4);
  std::copy(std::begin(mats5), std::end(mats5), std::begin(mats) + (*max_num_nucs) * 5);
  std::copy(std::begin(mats6), std::end(mats6), std::begin(mats) + (*max_num_nucs) * 6);
  std::copy(std::begin(mats7), std::end(mats7), std::begin(mats) + (*max_num_nucs) * 7);
  std::copy(std::begin(mats8), std::end(mats8), std::begin(mats) + (*max_num_nucs) * 8);
  std::copy(std::begin(mats9), std::end(mats9), std::begin(mats) + (*max_num_nucs) * 9);
  std::copy(std::begin(mats10), std::end(mats10), std::begin(mats) + (*max_num_nucs) * 10);
  std::copy(std::begin(mats11), std::end(mats11), std::begin(mats) + (*max_num_nucs) * 11);

  return mats;
}

// Randomizes the concentrations of all nuclides in a variety of materials
std::vector<double> load_concs(const std::vector<int>& num_nucs, int max_num_nucs) {
  uint64_t seed = STARTING_SEED * STARTING_SEED;
  std::vector<double> concs(12 * max_num_nucs);

  for (int i = 0; i < 12; i++)
    for (int j = 0; j < num_nucs[i]; j++)
      concs[i * max_num_nucs + j] = LCG_random_double(&seed);

  return concs;
}