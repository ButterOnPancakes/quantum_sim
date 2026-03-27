#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

int get_nb(QuantumRegister *qreg) {
    for(int i = 0; i < qreg->size; i++) {
        if(cabs(qreg->array[i]) > EPSILON) return i;
    }
    return -1;
}

void main() {
    srand(time(NULL));

    int n = 5; int N = 1 << n;
    int maxa = 5;

    int a = rand()%(maxa-1)+1;
    while(gcd(a, N) != 1 || a == 1) a = rand()%(maxa-1)+1;

    for(int i = 0; i < N; i++) {
        QuantumRegister *qreg = qregister_create(n);
        qreg->array[0] = 0; qreg->array[i] = 1;
        apply_prod(qreg, 0, n, a, N);
        int res = get_nb(qreg);
        printf("%d * %d = %d [%d]\n", a, i, res, N);
    }
}