#include "../builder/circuit.h"
#include "../simulator/opti_sim.h"
#include "../../utils/utils.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>

QuantumRegister *create_bell_state() {
    QuantumRegister *qr = qregister_create(2);
    QuantumCircuit *qc = circuit_create(2);    
    add_unitary_gate(qc, 0, GATE_H, 0);
    circuit_execute(qc, qr, NULL, false);
    circuit_free(qc, false);
    return qr;
}

int main() {
    srand(time(NULL));
    QuantumRegister *sentqbit = qregister_create(1);
    QuantumRegister *bell_state = create_bell_state();

    // On suppose que Alice peut agir uniquement sur 0 et 1, et Bob sur 2 : Système global de qbits
    QuantumRegister *qbits = qregister_fuse(sentqbit, bell_state);
    
    qregister_free(sentqbit);
    qregister_free(bell_state);
    
    //Circuit d'Alice
    QuantumCircuit *alice_circuit = circuit_create(3);
    add_control_gate(alice_circuit, 0, 1, GATE_X, 0);
    add_unitary_gate(alice_circuit, 0, GATE_H, 0);
    add_measure(alice_circuit, 0, 0);
    add_measure(alice_circuit, 1, 1);

    ClassicalRegister *alice_reg = cregister_create(2);

    circuit_execute(alice_circuit, qbits, alice_reg, false);
    circuit_free(alice_circuit, false);

    //Tour de Bob, il n'a acces que aux bits mesurés et au 3e qbit
    QuantumCircuit *bob_circuit = circuit_create(3);
    if(alice_reg->bits[1] == 1) {
        add_unitary_gate(bob_circuit, 2, GATE_X, 0);
    }
    if(alice_reg->bits[0] == 1) {
        add_unitary_gate(bob_circuit, 2, GATE_Z, 0);
    }
    circuit_execute(bob_circuit, qbits, NULL, false);
    circuit_free(bob_circuit, false);

    cregister_free(alice_reg);

    // TODO : Moyen de split des qbits
    qregister_print(stdout, qbits);
    qregister_free(qbits);

    return EXIT_SUCCESS;
}