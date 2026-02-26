#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "../utils/list.h"

#include <stdbool.h>
#include <complex.h>

typedef enum { ID, X, Z, H, MEAS, CNOT, ORACLE, S0 } GateType;

typedef struct {
    GateType type;
    bool (*f)(int *t, int n); // Oracle function
    int params[3]; // 0 : target qbit, 1 : control qbit or bit measured
} Gate;

typedef struct {
    int nb_qbits;
    List* gates;
} QuantumCircuit;

double complex* init_zero_state(int nqubits);
double complex* fuse_qbits(double complex *q1, int n1, double complex *q2, int n2);

QuantumCircuit *circuit_create(int n_qbits);
void circuit_free(QuantumCircuit *circuit);

void circuit_print(QuantumCircuit *circuit);

void add_single_qbit_gate(QuantumCircuit *circuit, int row, GateType g);
void add_double_qbit_gate(QuantumCircuit *circuit, int row, int control, GateType g);
void add_single_qbit_measure(QuantumCircuit *circuit, int row, int output);
void add_multiple_qbit_gate(QuantumCircuit *circuit, int row, int nb_qbits, GateType g, bool (*f)(int *t, int n));

#endif