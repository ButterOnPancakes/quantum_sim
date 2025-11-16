#include "../builder/circuit.h"
#include "../simulator/opti/opti_sim.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include <time.h>

static void *aligned_alloc_64(size_t size) {
    void *p = NULL;
    // POSIX aligned_alloc (size must be multiple of alignment)
    size_t align = 64;
    size_t sz = ((size + align - 1) / align) * align;
    p = aligned_alloc(align, sz);
    if (!p) {
        // fallback to posix_memalign
        if (posix_memalign(&p, align, sz) != 0) p = NULL;
    }
    return p;
}
static double complex *state_alloc(int nqubits) {
    uint64_t dim = 1ULL << nqubits;
    double complex *s = aligned_alloc_64(dim * sizeof(double complex));
    if (!s) {
        // fallback
        s = malloc(dim * sizeof(double complex));
    }
    return s;
}
static void init_zero_state(double complex *s, int nqubits) {
    uint64_t dim = 1ULL << nqubits;
    for (uint64_t i = 0; i < dim; ++i) s[i] = 0.0 + 0.0*I;
    s[0] = 1.0 + 0.0*I;
}

void test_simple_gates(int n) {
    QuantumCircuit *qc = create_circuit(n);

    double complex *statevector = state_alloc(n);
    init_zero_state(statevector, n);

    for(int i = 0; i < n; i++) {
        add_single_qbit_gate(qc, i, H);
        //add_single_qbit_gate(qc, i, Z);
        //add_single_qbit_gate(qc, i, X);
    }

    int *bits = circuit_execute_opti(qc, statevector);
    print_list(bits, n);

    destroy_circuit(qc);
    free(bits);
    free(qc);

    free(statevector);
}

void test_bell_state(int n) {
    printf("----------------------- Bell State Circuit -----------------------\n\n");

    double complex* statevector = calloc((1ULL << n), sizeof(double complex));
    statevector[0] = 1 + 0 * I;

    QuantumCircuit *qc = create_circuit(n);

    add_single_qbit_gate(qc, 0, H);
    add_double_qbit_gate(qc, 1, 0, CNOT);
    add_single_qbit_measure(qc, 0, 0);
    add_single_qbit_measure(qc, 1, 1);

    print_circuit(qc);

    int *bits = circuit_execute_opti(qc, statevector);
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
    int *alice_bits = circuit_execute_opti(qc, statevector);
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
    int *bob_bits = circuit_execute_opti(bob_qc, statevector);

    destroy_circuit(bob_qc);
    free(bob_qc);

    free(statevector);

    free(alice_bits);
    free(bob_bits);
    printf("\n-------------------------------------------------------------------\n");
}

int main() {
    test_simple_gates(30);
    //test_bell_state(2);
    //test_teleportation();

    return 0;
}
