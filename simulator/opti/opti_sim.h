#ifndef OPTI_SIM_H
#define OPTI_SIM_H

#include "../../builder/new/circuit.h"

#include <complex.h>
#include <stdbool.h>

double circuit_execute(QuantumCircuit *circuit, bool log);

#endif