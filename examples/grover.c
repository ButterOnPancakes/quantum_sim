#include "../builder/registers.h"
#include "../simulator/operations.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <math.h>

double run_grover(QuantumRegister *qreg, int start_qbit, int nb_qbits, bool (*oracle)(int)) {
    double start_time = now_seconds();
    apply_n_hadamard(qreg, start_qbit, nb_qbits);

    uint64_t N = 1 << nb_qbits;
    uint64_t iterations = (int)(M_PI / 4.0 * sqrt((double)N));
    
    //printf("Running Grover for %d qubits (%lu states)...\n", nb_qbits, N);
    //printf("Optimal iterations: %lu\n", iterations);

    fflush(stdout);
    for (uint64_t i = 0; i < iterations; i++) {
        apply_oracle_phase(qreg, start_qbit, nb_qbits, oracle);
        apply_diffusion(qreg, start_qbit, nb_qbits);
        
        //printf("Iteration %lu/%lu complete.\n", i + 1, iterations);
    }

    double end_time = now_seconds();
    //printf("Grover's algorithm completed in %f seconds.\n", end_time - start_time);
    return end_time - start_time;
}

int target = 10;
bool oracle(int number) {
    //printf("Oracle called with input: %d\n", number);
    return number == target;
}

int main() {
    for(int nb_qbits = 1; nb_qbits <= 25; nb_qbits++) {
        QuantumRegister *qreg = qregister_create(nb_qbits);
        ClassicalRegister *creg = cregister_create(nb_qbits);

        run_grover(qreg, 0, nb_qbits, oracle);

        for(int i = 0; i < nb_qbits; i++) {
            qregister_measure(qreg, i, creg, i);
            //printf("Measured qubit %d with probability %f\n", i, proba);
        }
        printf("\n");

        printf("Measurement results: %ld\n", cregister_calc_number(creg));
        cregister_print(creg);
        printf("Expected result: %d\n", target);

        qregister_free(qreg);
        cregister_free(creg);
    }
    return 0;
}