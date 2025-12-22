#include "../../builder/new/circuit.h"
#include "../../simulator/opti/opti_sim.h"
#include "../../utils/utils.h"

#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    ClassicalRegister *cregister = create_cregister(3);
    QuantumRegister *qregister = create_qregister(3);
    QuantumCircuit *qc = create_circuit(qregister, cregister);

    add_unitary_gate(qc, 0, GATE_H);
    add_control_gate(qc, 0, 1, GATE_X);
    add_custom_gate(qc, 2, (int[]){1,2}, (double complex[]){
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1,
        0, 0, 1, 0
    }, "SWAP");
    add_measure(qc, 0, 0);
    add_measure(qc, 1, 1);
    add_measure(qc, 2, 2);

    print_circuit(qc);

    circuit_execute(qc);

    print_qregister(qc->qregister);
    print_cregister(qc->cregister);

    destroy_circuit(qc);
    free_cregister(cregister);
    free_qregister(qregister);

    return EXIT_SUCCESS;
}