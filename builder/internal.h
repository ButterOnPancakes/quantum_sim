#ifndef INTERNAL_H
#define INTERNAL_H

#include <complex.h>
#include "../utils/list.h"
#include "gaterep.h"

// Note: SingleBitGate is exposed in gaterep.h

struct ClassicalRegister {
    int *bits;
    int nb_bits;
};

struct QuantumRegister {
    double complex *statevector;
    int nb_qbits;
};

struct QuantumCircuit {
    int nb_qbits;
    List* gates;
};

struct Gate {
    enum {MEAS, UNITARY, CONTROL, CUSTOM} class;
    union {
        struct {
            int qbit;
            int cbit;
        } measure;
        struct {
            int qbit;
            SingleBitGate type;
            double phase; //exp(I * phase)
        } unitary;
        struct {
            int control;
            int qbit;
            SingleBitGate type;
            double phase; //exp(I * phase)
        } control;
        struct {
            int nb_qbits;
            int *qbits;
            double complex *mat;
            char *label;
        } custom;
    } gate;
};

#endif
