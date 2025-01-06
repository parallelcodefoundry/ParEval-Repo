// microXOR header file

#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>
#include <Kokkos_Core.hpp>

void cellsXOR(const Kokkos::View<const int*> input, Kokkos::View<int*> output, size_t N);

#endif