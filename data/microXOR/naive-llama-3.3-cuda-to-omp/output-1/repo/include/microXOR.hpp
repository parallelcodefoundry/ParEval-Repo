// microXOR header file

#ifndef MICROXOR_H
#define MICROXOR_H

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void cellsXOR(const int *input, int *output, size_t N);

#endif