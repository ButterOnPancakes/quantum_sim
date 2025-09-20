#include "circuit.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    // Test 1: 2-qubit circuit with H, CNOT, and measurement
    QuantumCircuit *qc1 = create_circuit(2);
    add_single_qbit_gate(qc1, 0, H);        // H on qubit 0 at time 0
    add_double_qbit_gate(qc1, 1, 0, CNOT);  // CNOT(control=1, target=0) at time 1
    add_single_qbit_measure(qc1, 0, 0);     // Measure qubit 0 → output 0 at time 2
    printf("Test 1 - 2-qubit Bell circuit:\n");
    print_circuit(qc1);
    destroy_circuit(qc1);

    // Test 2: 3-qubit circuit with parallel X and H
    QuantumCircuit *qc2 = create_circuit(3);
    add_single_qbit_gate(qc2, 1, X);        // X on qubit 1 at time 0
    add_single_qbit_gate(qc2, 0, H);        // H on qubit 0 at time 1
    add_double_qbit_gate(qc2, 0, 2, CNOT);  // CNOT(control=2, target=0) at time 2
    add_single_qbit_measure(qc2, 0, 0);     // Measure qubit 0 → output 0 at time 3
    add_single_qbit_measure(qc2, 2, 1);     // Measure qubit 2 → output 1 at time 4
    printf("\nTest 2 - 3-qubit circuit:\n");
    print_circuit(qc2);
    destroy_circuit(qc2);

    return 0;
}