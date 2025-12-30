#ifndef DAGCIRCUIT_H
#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "register.h"
#include "gaterep.h"
#include "../utils/list.h"

#include <complex.h>
#include <stdio.h>

typedef struct {
    QuantumRegister *qregister;
    ClassicalRegister *cregister;
    List* gates;
} QuantumCircuit;

QuantumCircuit *circuit_create(QuantumRegister *qregister, ClassicalRegister *cregister);
void circuit_free(QuantumCircuit *circuit);

void circuit_print(FILE *channel, QuantumCircuit *circuit);

void add_unitary_gate(QuantumCircuit *circuit, int t, SingleBitGate tg, double phase);
void add_control_gate(QuantumCircuit *circuit, int c, int t, SingleBitGate tg, double phase);
// Mat size must be 2^nb_qbits !
void add_custom_gate(QuantumCircuit *circuit, int nb_qbits, int *t, double complex *mat, char *label);
void add_measure(QuantumCircuit *circuit, int qbit, int cbit);

#endif
#endif