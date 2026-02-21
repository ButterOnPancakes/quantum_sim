#include "../builder/circuit_old.h"
#include "../simulator/base_sim.h"
#include "../../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include <omp.h>

#include <time.h>

void create_bell_state(double complex* zero_state, int n) {
    QuantumCircuit *qc = circuit_create(n);

    add_single_qbit_gate(qc, 0, H);
    add_double_qbit_gate(qc, 1, 0, CNOT);

    int *bits = NULL;
    circuit_execute(qc, zero_state, &bits);

    circuit_free(qc);
    free(qc);
    free(bits);
}

// Teleports the first qbit
void test_teleportation() {
    printf("\n--------------------- Teleportation Circuit ----------------------\n\n");

    // Inits the circuit
    double complex transmitted[2] = {1, 0};
    double complex bell_state[4] = {1, 0, 0, 0};

    create_bell_state(bell_state, 2);

    double complex *final_state = fuse_qbits(transmitted, 1, bell_state, 2);

    // Alice part
    QuantumCircuit *alice_qc = circuit_create(3);

    add_double_qbit_gate(alice_qc, 1, 0, CNOT);
    add_single_qbit_gate(alice_qc, 0, H);
    add_single_qbit_measure(alice_qc, 0, 0);
    add_single_qbit_measure(alice_qc, 1, 1);

    int *alice_bits = NULL;
    circuit_execute(alice_qc, final_state, &alice_bits);

    circuit_free(alice_qc); free(alice_qc);

    QuantumCircuit *bob_qc = circuit_create(3);
    // Bob's turn, he only has 
    if(alice_bits[0] == 0 && alice_bits[1] == 0) {
        add_single_qbit_gate(bob_qc, 2, ID);
    }
    if(alice_bits[0] == 0 && alice_bits[1] == 1) {
        add_single_qbit_gate(bob_qc, 2, X);
    }
    if(alice_bits[0] == 1 && alice_bits[1] == 0) {
        add_single_qbit_gate(bob_qc, 2, Z);
    }
    if(alice_bits[0] == 1 && alice_bits[1] == 1) {
        add_single_qbit_gate(bob_qc, 2, X);
        add_single_qbit_gate(bob_qc, 2, Z);
    }
    add_single_qbit_measure(bob_qc, 2, 2);

    int *bob_bits = NULL;
    circuit_execute(bob_qc, final_state, &bob_bits);

    circuit_print(bob_qc);
    
    print_int_array(alice_bits, 2);
    print_int_array(&bob_bits[2], 1);

    free(alice_bits);
    free(bob_bits);
    
    free(final_state);

    circuit_free(bob_qc);
    free(bob_qc);

    printf("\n-------------------------------------------------------------------\n");
}

int main() {
    srand(time(NULL));
    
    test_teleportation();

    return 0;
}
