#ifndef OPTI_SIM_H
#define OPTI_SIM_H

#include "../builder/circuit.h"
#include "../builder/register.h"

#include <complex.h>
#include <stdbool.h>

double circuit_execute(QuantumCircuit *circuit, QuantumRegister *qregister, ClassicalRegister *cregister, bool log);

#endif