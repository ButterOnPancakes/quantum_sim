#ifndef EMMS_SIM_H
#define EMMS_SIM_H

#include "../builder/circuit.h"
#include <complex.h>
#include <stdbool.h>

/**
 * Computes the statevector output of the circuit assuming the initial state is |0...0>.
 */
double complex *emms_compute_statevector(QuantumCircuit *circuit);

#endif
