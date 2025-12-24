#include "gaterep.h"

#include <stdlib.h>

Gate *create_unitary_gate(int t, SingleBitGate tg) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = UNITARY;
    gate->gate.unitary.type = tg;
    gate->gate.unitary.qbit = t;
    return gate;
}
Gate *create_control_gate(int c, int t, SingleBitGate tg) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = CONTROL;
    gate->gate.control.control = c;
    gate->gate.control.qbit = t;
    gate->gate.control.type = tg;
    return gate;
}
// Mat size must be 2^nb_qbits !
Gate *create_custom_gate(int nb_qbits, int *t, double complex *mat, char *label) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = CUSTOM;
    gate->gate.custom.nb_qbits = nb_qbits;
    gate->gate.custom.qbits = t;
    gate->gate.custom.mat = mat;
    gate->gate.custom.label = label;
    return gate;
}
Gate *create_measure(int qbit, int cbit) {
    Gate *gate = malloc(sizeof(Gate));
    gate->class = MEAS;
    gate->gate.measure.cbit = cbit;
    gate->gate.measure.qbit = qbit;
    return gate;
}