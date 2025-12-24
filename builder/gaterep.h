#ifndef GATEREP_H
#define GATEREP_H

#include <complex.h>

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

Gate *create_unitary_gate(int t, SingleBitGate tg);
Gate *create_control_gate(int c, int t, SingleBitGate tg);
// Mat size must be 2^nb_qbits !
Gate *create_custom_gate(int nb_qbits, int *t, double complex *mat, char *label);
Gate *create_measure(int qbit, int cbit);

#endif