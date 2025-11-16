#include "opti_sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <omp.h>

#include "gates.h"

#include "../../utils/list.h"
#include "../../utils/utils.h"

int *circuit_execute_opti(QuantumCircuit *circuit, double complex *state) {
    assert(circuit != NULL);
    srand(time(NULL));

    int *bits = calloc(circuit->nb_qbits, sizeof(int));

    int nthreads = omp_get_max_threads();
    omp_set_num_threads(nthreads);
    printf("OpenMP enabled: threads=%d\n", nthreads);

    double t0 = now_seconds();
    
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
        GateType gt = gate->type;
        int qbit = gate->params[0];

        double complex gm[4] = {1, 0, 1, 0};
        switch (gt) {
            case H: gate_h(gm);
            case Z: gate_z(gm);
            case X: gate_x(gm);
            
            default:
                apply_single_qubit_inplace(state, circuit->nb_qbits, qbit, gm);
                break;
        }
    }

    double t1 = now_seconds();
    printf("Execution Time : %.6f s\n", t1 - t0);

    return bits;
}
