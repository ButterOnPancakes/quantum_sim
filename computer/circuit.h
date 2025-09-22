#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "../utils/dynarrays/dynarray.h"

typedef enum { ID, X, H, CNOT } GateType;

typedef struct {
    GateType gt;
    int params[];
} Gate;

typedef struct {
    enum { GATE, MEASURE } type;
    union {
        Gate gate;
        struct {
            int qbit;
            int bit;
        } measure;
    } operation;
} Instruction;

typedef struct qcircuit {
    int n_qbits;
    DynArray *instructions;
} QuantumCircuit;

QuantumCircuit *create_circuit(int n_qbits);
int destroy_circuit(QuantumCircuit *circuit);

void print_circuit(QuantumCircuit *circuit);

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g);
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g);
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output);

#endif