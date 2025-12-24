#ifndef BASE_SIM_H
#define BASE_SIM_H

#include "../builder/circuit_old.h"

#include <complex.h>

int *circuit_execute(QuantumCircuit *circuit, double complex *statevector_mat);

#endif