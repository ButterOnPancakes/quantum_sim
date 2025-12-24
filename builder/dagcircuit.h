#ifndef CIRCUIT_H
#ifndef DAGCIRCUIT_H
#define DAGCIRCUIT_H

#include <complex.h>

#include "gaterep.h"
#include "register.h"

typedef struct node_s Node;

typedef struct {
    Node *root; //root of the DAG, placeholder node with no gate
    Node **last_used; //last_used[i] : last gate that used qubit i
    QuantumRegister *qreg;
    ClassicalRegister *creg;
} DAGCircuit;

DAGCircuit* dagcircuit_create(QuantumRegister* qreg, ClassicalRegister* creg);
void dagcircuit_free(DAGCircuit* circuit);

void dagcircuit_add_unitary_gate(DAGCircuit *circuit, int t, SingleBitGate tg);
void dagcircuit_add_control_gate(DAGCircuit *circuit, int c, int t, SingleBitGate tg);
// Mat size must be 2^nb_qbits !
void dagcircuit_add_custom_gate(DAGCircuit *circuit, int nb_qbits, int *t, double complex *mat, char *label);
void dagcircuit_add_measure(DAGCircuit *circuit, int qbit, int cbit);

#endif
#endif