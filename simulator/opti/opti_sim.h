#ifndef OPTI_SIM_H
#define OPTI_SIM_H

#include "../../builder/circuit.h"

#include <complex.h>

int *circuit_execute_opti(QuantumCircuit *circuit, double complex *statevector_mat);

#endif