#include "../builder/circuit.h"
#include "../simulator/naive/base_sim.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <time.h>

void test_bell_state(int n) {
    printf("----------------------- Bell State Circuit -----------------------\n\n");

    double complex* statevector = calloc((1ULL << n) * 1, sizeof(double complex));
    statevector[0] = 1 + 0 * I;

    QuantumCircuit *qc = create_circuit(n);

    add_single_qbit_gate(qc, 0, H);
    add_double_qbit_gate(qc, 1, 0, CNOT);
    add_single_qbit_measure(qc, 0, 0);
    add_single_qbit_measure(qc, 1, 1);

    print_circuit(qc);

    int *bits = circuit_execute(qc, statevector);
    print_list(bits, n);

    destroy_circuit(qc);
    free(bits);
    free(qc);

    free(statevector);

    printf("\n------------------------------------------------------------------\n");
}

// Teleports the first qbit
void test_teleportation() {
    printf("\n--------------------- Teleportation Circuit ----------------------\n\n");

    // Inits the circuit
    int n = 3;
    double complex st[2][1] = {{1}, {0}}; // State to teleport
    double complex id[4][1] = {{1}, {0}, {0}, {0}};
    double complex *statevector = calloc(8, sizeof(double complex));
    // Kronecker product of st and |00>
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 4; j++) {
            statevector[i * 4 + j] = st[i][0] * id[j][0];
        }
    }
    // Normalization
    double norm = 0.0;
    for(int i = 0; i < 8; i++) {
        norm += creal(statevector[i]) * creal(statevector[i]) + cimag(statevector[i]) * cimag(statevector[i]);
    }
    norm = sqrt(norm);
    for(int i = 0; i < 8; i++) {
        statevector[i] /= norm;
    }

    QuantumCircuit *qc = create_circuit(n);

    // Bell State Creation
    add_single_qbit_gate(qc, 1, H);
    add_double_qbit_gate(qc, 2, 1, CNOT);

    // Alice's measurements
    add_double_qbit_gate(qc, 1, 0, CNOT);
    add_single_qbit_gate(qc, 0, H);
    add_single_qbit_measure(qc, 0, 0);
    add_single_qbit_measure(qc, 1, 1);

    print_circuit(qc);
    
    // Only takes the first 2 in account
    int *alice_bits = circuit_execute(qc, statevector);
    print_list(alice_bits, 2);
    
    destroy_circuit(qc);
    free(qc);
    qc = NULL;

    QuantumCircuit *bob_qc = create_circuit(n);
    if(alice_bits[0] == 0 && alice_bits[1] == 0) { // Identity
        add_single_qbit_gate(bob_qc, 2, ID);
    }
    else if(alice_bits[0] == 0 && alice_bits[1] == 1) { // X gate
        add_single_qbit_gate(bob_qc, 2, X);
    }
    else if(alice_bits[0] == 1 && alice_bits[1] == 0) { // Z gate
        add_single_qbit_gate(bob_qc, 2, Z);
    }
    else { // X then Z gates
        add_single_qbit_gate(bob_qc, 2, X);
        add_single_qbit_gate(bob_qc, 2, Z);
    }
    
    print_circuit(bob_qc);

    // Takes the third (teleported qbit) in account
    int *bob_bits = circuit_execute(bob_qc, statevector);

    destroy_circuit(bob_qc);
    free(bob_qc);

    free(statevector);

    free(alice_bits);
    free(bob_bits);
    printf("\n-------------------------------------------------------------------\n");
}

int main() {
    test_bell_state(2);
    test_teleportation();

    return 0;
}
