#include "../builder/circuit.h"
#include "../builder/operations.h"
#include "../simulator/emms_sim.h"

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

int main() {
    printf("Testing EMMS Parallel Simulator...\n");

    QuantumCircuit *qc = circuit_create(2);
    circuit_add_h_gate(qc, 0);
    circuit_add_cnot_gate(qc, 0, 1);

    printf("Circuit for Bell state (Optimizing...):\n");
    circuit_optimize(qc);
    print_tree(qc->root, 0);

    printf("\nComputing statevector...\n");
    double complex *sv = emms_compute_statevector(qc);

    printf("\nResulting Statevector:\n");
    for (int i = 0; i < 4; i++) {
        printf("|%d>: %.4f + %.4fi\n", i, creal(sv[i]), cimag(sv[i]));
    }

    // Expected Bell state: 1/sqrt(2) (|00> + |11>)
    double expected = 1.0 / sqrt(2.0);
    printf("\nExpected: |00>: %.4f, |11>: %.4f\n", expected, expected);

    free(sv);
    circuit_free(qc);

    return 0;
}