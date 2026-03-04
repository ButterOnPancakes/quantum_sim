#ifndef EMMS_SIM_H
#define EMMS_SIM_H

#include "../builder/circuit.h"
#include <complex.h>
#include <stdbool.h>

/**
 * Computes the statevector output of the circuit
 */
void emms_compute_statevector(QuantumCircuit *circuit, double complex* vector, uint64_t dim);

#endif
