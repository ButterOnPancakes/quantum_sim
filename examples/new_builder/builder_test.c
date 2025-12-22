#include "../../builder/new/circuit.h"
#include "../../simulator/opti/opti_sim.h"
#include "../../utils/utils.h"

#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    ClassicalRegister *cregister = create_cregister(2);
    QuantumRegister *qregister = create_qregister(3);
    QuantumCircuit *qc = create_circuit(qregister, cregister);

    add_unitary_gate(qc, 0, GATE_H);
    add_control_gate(qc, 0, 1, GATE_X);
    add_control_gate(qc, 1, 2, GATE_X);
    add_measure(qc, 0, 0);
    add_measure(qc, 1, 1);

    print_circuit(qc);

    circuit_execute(qc);

    print_qregister(qc->qregister);
    print_cregister(qc->cregister);

    destroy_circuit(qc);
    free_cregister(cregister);
    free_qregister(qregister);

    return EXIT_SUCCESS;
}