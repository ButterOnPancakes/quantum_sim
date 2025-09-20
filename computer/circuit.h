#ifndef CIRCUIT_H
#define CIRCUIT_H

typedef enum { I, X, H, CNOT } GateType;

typedef struct qcircuit QuantumCircuit;

QuantumCircuit *create_circuit(int n_qbits);
int destroy_circuit(QuantumCircuit *circuit);

void print_circuit(QuantumCircuit *circuit);

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g);
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g);
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output);

#endif