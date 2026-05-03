#include "qft.h"

#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Reversed because LSB convention
void qft(QuantumRegister *qreg) {
    for(int j = qreg->nb_qbits - 1; j >= 0; j--) {
        apply_hadamard(qreg, j);
        double phase = M_PI / 2;
        for(int i = j - 1; i >= 0; i--) {
            apply_controlled_rotation(qreg, i, j, phase);
            phase /= 2;
        }
    }

    for(int i = 0; i < qreg->nb_qbits / 2; i++) {
        apply_swap(qreg, i, qreg->nb_qbits - i - 1);
    }
}