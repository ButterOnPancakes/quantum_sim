#ifndef GATEREP_H
#define GATEREP_H

#include <complex.h>

typedef enum {
    GATE_I,
    GATE_H,
    GATE_X,
    GATE_Y,
    GATE_Z,
    GATE_PHASE
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
} Gate;

Gate *create_unitary_gate(int t, SingleBitGate tg, double phase);
Gate *create_control_gate(int c, int t, SingleBitGate tg, double phase);
// Mat size must be 2^nb_qbits !
Gate *create_custom_gate(int nb_qbits, int *t, double complex *mat, char *label);
Gate *create_measure(int qbit, int cbit);

#endif