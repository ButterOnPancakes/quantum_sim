#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdlib.h>
#include <stdio.h>

double qft_execution(int n) {
    QuantumRegister *qreg = qregister_create(n);

    double start = now_seconds();
    apply_qft(qreg, 0, n);
    double end = now_seconds();

    return end - start;
}

int main(int argc, char *argv[]) {
    int nb_qbits = 10;
    if(argc >= 2) {
       nb_qbits = atoi(argv[1]);
    }
    printf("%f\n", qft_execution(nb_qbits));
    return 0;
}