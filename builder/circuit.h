#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "emms.h"

typedef struct {
    int nb_qbits;
    Node *root;
} QuantumCircuit;

QuantumCircuit *circuit_create(int nb_qbits);
void circuit_free(QuantumCircuit *qc);

void circuit_optimize(QuantumCircuit *qc);

void circuit_add_x_gate(QuantumCircuit *qc, int target);
void circuit_add_y_gate(QuantumCircuit *qc, int target);
void circuit_add_z_gate(QuantumCircuit *qc, int target);
void circuit_add_h_gate(QuantumCircuit *qc, int target);

void circuit_add_cnot_gate(QuantumCircuit *qc, int control, int target);
void circuit_add_cz_gate(QuantumCircuit *qc, int control, int target);
void circuit_add_cphase_gate(QuantumCircuit *qc, int control, int target, double theta);
void circuit_add_swap_gate(QuantumCircuit *qc, int qbit1, int qbit2);

#endif