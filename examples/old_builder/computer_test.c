#include "../../builder/old/circuit_old.h"
#include "../../simulator/naive/base_sim.h"
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
    QuantumCircuit *qc = create_circuit(n);

    add_single_qbit_gate(qc, 0, H);
    add_double_qbit_gate(qc, 1, 0, CNOT);

    int *bits = circuit_execute(qc, zero_state);

    destroy_circuit(qc);
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
    QuantumCircuit *alice_qc = create_circuit(3);

    add_double_qbit_gate(alice_qc, 1, 0, CNOT);
    add_single_qbit_gate(alice_qc, 0, H);
    add_single_qbit_measure(alice_qc, 0, 0);
    add_single_qbit_measure(alice_qc, 1, 1);

    //Execute_opti does not work, maybe it has to do with bit order
    int *alice_bits = circuit_execute(alice_qc, final_state);

    destroy_circuit(alice_qc); free(alice_qc);

    QuantumCircuit *bob_qc = create_circuit(3);
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

    //Execute_opti does not work, maybe it has to do with bit order
    int *bob_bits = circuit_execute(bob_qc, final_state);

    print_circuit(bob_qc);
    
    print_list(alice_bits, 2);
    print_list(&bob_bits[2], 1);

    free(alice_bits);
    free(bob_bits);
    
    free(final_state);

    destroy_circuit(bob_qc);
    free(bob_qc);

    printf("\n-------------------------------------------------------------------\n");
}

int main() {
    srand(time(NULL));
    int nthreads = omp_get_max_threads();
    omp_set_num_threads(nthreads);
    printf("OpenMP enabled: threads=%d\n", nthreads);
    
    test_teleportation();

    return 0;
}
