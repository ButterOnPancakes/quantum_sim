#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

int main() {
    srand(time(NULL));
    
    QuantumRegister *fst = qregister_create(1);
    qregister_set_number(fst, 1);
    QuantumRegister *snd = qregister_create(1);
    qregister_set_number(snd, 0);

    QuantumRegister *qreg = qregister_fuse(fst, snd);
    qregister_print(qreg);

    printf("\n\n");

    apply_gate_x(qreg, 1);
    qregister_print(qreg);

    return EXIT_SUCCESS;
}