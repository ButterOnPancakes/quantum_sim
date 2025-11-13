#include "../computer/circuit.h"
#include "../matrices/matrices.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <time.h>

void test_bell_state(int n) {
    printf("----------------------- Bell State Circuit -----------------------\n\n");

    Matrix *statevector = matrix_zero(fast_exp_i(2, n), 1);
    matrix_set(statevector, 0, 0, 1.0);

    QuantumCircuit *qc = create_circuit(n);

    add_single_qbit_gate(qc, 0, H);
    add_double_qbit_gate(qc, 1, 0, CNOT);
    add_single_qbit_measure(qc, 0, 0);
    add_single_qbit_measure(qc, 1, 1);

    print_circuit(qc);

    int *bits = circuit_execute(qc, &statevector);
    print_list(bits, n);

    destroy_circuit(qc);
    free(bits);
    free(qc);

    matrix_free(statevector);

    printf("\n------------------------------------------------------------------\n");
}

// Teleports the first qbit
void test_teleportation() {
    printf("\n--------------------- Teleportation Circuit ----------------------\n\n");

    // Inits the circuit
    int n = 3;
    double complex st[2][1] = {{2 + 3 * I}, {-4 + 0.1 * I}};
    double complex id[4][1] = {{1}, {0}, {0}, {0}};
    Matrix *statevector = matrix_of_array(2, 1, st);
    matrix_tensor_product_stack(statevector, 4, 1, id, &statevector);
    matrix_normalise(statevector);

    matrix_print(statevector);
    
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
    int *alice_bits = circuit_execute(qc, &statevector);
    print_list(alice_bits, 2);
    
    destroy_circuit(qc);
    free(qc);
    qc = NULL;

    QuantumCircuit *bob_qc = create_circuit(n);
    if(alice_bits[0] == 0 && alice_bits[1] == 0) { // Identity
        add_single_qbit_gate(bob_qc, 2, ID);
    }
    else if(alice_bits[0] == 0 && alice_bits[1] == 1) { // X gate
        add_single_qbit_gate(bob_qc, 2, XGATE);
    }
    else if(alice_bits[0] == 1 && alice_bits[1] == 0) { // Z gate
        add_single_qbit_gate(bob_qc, 2, ZGATE);
    }
    else { // X then Z gates
        add_single_qbit_gate(bob_qc, 2, XGATE);
        add_single_qbit_gate(bob_qc, 2, ZGATE);
    }
    
    print_circuit(bob_qc);

    // Takes the third (teleported qbit) in account
    int *bob_bits = circuit_execute(bob_qc, &statevector);

    matrix_print(statevector);

    destroy_circuit(bob_qc);
    free(bob_qc);

    free(alice_bits);
    free(bob_bits);
    matrix_free(statevector);
    printf("\n-------------------------------------------------------------------\n");
}

int main() {
    test_bell_state(2);
    test_teleportation();

    return 0;
}
