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
#include "../../utils/logger.h"

double circuit_execute(QuantumCircuit *circuit, bool log) {
    double t0 = now_seconds();

    Logger *logger = NULL;
    FILE *log_file = NULL;
    if(log) {
        log_file = create_log_file("circuit_execution.log");
        print_circuit(log_file, circuit);

        logger = create_logger();
        set_channel(logger, log_file, true);
        logMessage(logger, "INFO", "Starting circuit execution.");
    }
    
    double complex gm[4] = {1, 0, 0, 1};

    char buffer[1024];
    
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        switch (gate->class) {
            case UNITARY: 
                if(log) sprintf(buffer, "Applying unitary gate on qubit %d.", gate->gate.unitary.qbit);
                apply_corresponding_gate(gm, gate->gate.unitary.type);
                apply_single_qubit_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.unitary.qbit, 
                    gm
                );
                break;
            
            case CONTROL:
                if(log) sprintf(buffer, "Applying controlled gate with control qubit %d and target qubit %d.", gate->gate.control.control, gate->gate.control.qbit);
                apply_corresponding_gate(gm, gate->gate.control.type);
                apply_controlled_u_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.control.control, gate->gate.control.qbit, 
                    gm
                ); 
                break;
            
            case CUSTOM:
                if(log) sprintf(buffer, "Applying custom gate on %d qubits.", gate->gate.custom.nb_qbits);
                apply_custom_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.custom.qbits, gate->gate.custom.nb_qbits, 
                    gate->gate.custom.mat
                );
                break;
            
            case MEAS:
                if(log) sprintf(buffer, "Measuring qubit %d into classical bit %d.", gate->gate.measure.qbit, gate->gate.measure.cbit);
                circuit->cregister->bits[gate->gate.measure.cbit] = measure_qubit_inplace(
                    circuit->qregister->statevector, circuit->qregister->nb_qbits, 
                    gate->gate.measure.qbit
                );
                break;

            default:
                if(log) sprintf(buffer, "Unknown gate class encountered.");
                break;
        }
        if(log) logMessage(logger, "INFO", buffer);
    }

    double t1 = now_seconds();
    if(log) {
        logMessage(logger, "INFO", "Circuit execution completed.");
        logMessage(logger, "INFO", "Final statevector:");
        print_qregister(log_file, circuit->qregister);
        logMessage(logger, "INFO", "Classical register contents:");
        print_cregister(log_file, circuit->cregister);
        close_logger(logger);
    }
    //printf("Execution Time : %.6f s\n", t1 - t0);
    return t1 - t0;
}
