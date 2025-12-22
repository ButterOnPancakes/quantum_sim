#include "opti_sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#include <omp.h>

#include "gates.h"

#include "../../utils/list.h"
#include "../../utils/utils.h"

void circuit_execute(QuantumCircuit *circuit) {
    double t0 = now_seconds();
    
    double complex gm[4] = {1, 0, 0, 1};
    
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        switch (gate->class) {
            case UNITARY: 
                apply_corresponding_gate(gm, gate->gate.unitary.type);
                apply_single_qubit_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.unitary.qbit, 
                    gm
                );
                break;
            
            case CONTROL:
                apply_corresponding_gate(gm, gate->gate.control.type);
                apply_controlled_u_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.control.control, gate->gate.control.qbit, 
                    gm
                ); 
                break;
            
            case MEAS:
                circuit->cregister->bits[gate->gate.measure.cbit] = measure_qubit_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.measure.qbit
                );
                break;
            
            case CUSTOM:
                apply_custom_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.custom.qbits, gate->gate.custom.nb_qbits, 
                    gate->gate.custom.mat
                );
                break;

            default: break;
        }
    }

    double t1 = now_seconds();
    printf("Execution Time : %.6f s\n", t1 - t0);
}
