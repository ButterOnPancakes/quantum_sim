#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <stdbool.h>
#include "../matrices/matrices.h"

typedef enum { IDENTITY, XGATE, ZGATE, HADAMARD, PROJ0, PROJ1 } Operator;
typedef enum { ID, X, Z, H, MEAS, CNOT, CZ, ORACLE, S0 } GateType;

typedef struct qcircuit QuantumCircuit;

QuantumCircuit *create_circuit(int n_qbits);
void destroy_circuit(QuantumCircuit *circuit);

void print_circuit(QuantumCircuit *circuit);

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g);
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g);
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output);
void add_multiple_qbit_gate(QuantumCircuit *circuit, int row, int nb_qbits, GateType g, bool (*f)(int *t, int n));

int *circuit_execute(QuantumCircuit *circuit, Matrix **entry);

#endif