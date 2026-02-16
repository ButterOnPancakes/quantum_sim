#ifndef BASE_SIM_H
#define BASE_SIM_H

#include "../builder/circuit_old.h"

#include <complex.h>

double circuit_execute(QuantumCircuit *circuit, double complex *statevector_mat, int **bits_out);

#endif