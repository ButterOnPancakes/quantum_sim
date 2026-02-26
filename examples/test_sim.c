#include "../builder/emms.h"
#include "../simulator/emms_sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

int main() {
    printf("Testing EMMS Parallel Simulator...\n");

    // Create a Bell state: H(0), CNOT(0, 1)
    Node *circuit = create_circuit(2);
    circuit = add_gate_to_circuit(circuit, create_h_gate(), 0);
    circuit = add_cnot_to_circuit(circuit, 0, 1);

    printf("Circuit for Bell state (Optimizing...):\n");
    circuit = full_optimize(circuit);
    print_tree(circuit, 0);

    printf("\nComputing statevector...\n");
    double complex *sv = emms_compute_statevector(circuit);

    printf("\nResulting Statevector:\n");
    for (int i = 0; i < 4; i++) {
        printf("|%d>: %.4f + %.4fi\n", i, creal(sv[i]), cimag(sv[i]));
    }

    // Expected Bell state: 1/sqrt(2) (|00> + |11>)
    double expected = 1.0 / sqrt(2.0);
    printf("\nExpected: |00>: %.4f, |11>: %.4f\n", expected, expected);

    free(sv);
    free_node(circuit, true);

    return 0;
}