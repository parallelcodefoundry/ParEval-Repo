// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <vector>
#include <Kokkos_Core.hpp>

void cellsXOR(Kokkos::View<const int**> &input, Kokkos::View<int**> &output, size_t N, size_t threadsPerTeam, size_t numTeams);

#endif
