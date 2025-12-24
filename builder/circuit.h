#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "../utils/list.h"

#include <complex.h>
#include <stdio.h>

typedef enum {
    GATE_I,
    GATE_H,
    GATE_X,
    GATE_Y,
    GATE_Z
} SingleBitGate;

typedef struct {
    enum {MEAS, UNITARY, CONTROL, CUSTOM} class;
    union {
        struct {
            int qbit;
            int cbit;
        } measure;
        struct {
            int qbit;
            SingleBitGate type;
        } unitary;
        struct {
            int control;
            int qbit;
            SingleBitGate type;
        } control;
        struct {
            int nb_qbits;
            int *qbits;
            double complex *mat;
            char *label;
        } custom;
    } gate;
} Gate;

typedef struct {
    int *bits;
    int nb_bits;
} ClassicalRegister;

typedef struct {
    double complex *statevector;
    int nb_qbits;
} QuantumRegister;

typedef struct {
    QuantumRegister *qregister;
    ClassicalRegister *cregister;
    List* gates;
} QuantumCircuit;

ClassicalRegister *create_cregister(int nbits);
void print_cregister(FILE *channel, ClassicalRegister *cregister);
void free_cregister(ClassicalRegister *cregister);

QuantumRegister *create_qregister(int nqubits);
QuantumRegister *fuse_qregister(QuantumRegister *q1, QuantumRegister *q2);
void print_qregister(FILE *channel, QuantumRegister *qregister);
void free_qregister(QuantumRegister *qregister);

QuantumCircuit *create_circuit(QuantumRegister *qregister, ClassicalRegister *cregister);
void destroy_circuit(QuantumCircuit *circuit);

void print_circuit(FILE *channel, QuantumCircuit *circuit);

void add_unitary_gate(QuantumCircuit *circuit, int t, SingleBitGate tg);
void add_control_gate(QuantumCircuit *circuit, int c, int t, SingleBitGate tg);
// Mat size must be 2^nb_qbits !
void add_custom_gate(QuantumCircuit *circuit, int nb_qbits, int *t, double complex *mat, char *label);

void add_measure(QuantumCircuit *circuit, int qbit, int cbit);
#endif