#include "opti_sim.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "../../utils/list.h"

int *circuit_execute(QuantumCircuit *circuit, double complex *statevector_mat) {
    assert(circuit != NULL);
    srand(time(NULL));

    int *bits = calloc(circuit->nb_qbits, sizeof(int));
    
    ListIterator iter = list_iterator_begin(circuit->gates);
    while (list_iterator_has_next(&iter)) {
        Gate *gate = list_iterator_next(&iter);
    }
    return NULL;
}
