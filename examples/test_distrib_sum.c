#include "../builder/emms.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

int main() {
    printf("Testing distrib_sum...\n");

    // Create a simple circuit with a CNOT gate
    // CNOT on qubit 0 (control) and 1 (target)
    Node *circuit = create_circuit(2);
    circuit = add_cnot_to_circuit(circuit, 0, 1);

    printf("Initial Circuit with CNOT:\n");
    print_tree(circuit, 0);

    // The CNOT is represented as a SUM of two TENSORs.
    // circuit is PRODUCT(SUM(TENSOR(P0, I), TENSOR(P1, X)), I_tree(2))

    Node *optimized = full_optimize(circuit);
    printf("\nOptimized Circuit:\n");
    print_tree(optimized, 0);

    printf("\nTests passed!\n");
    return 0;
}