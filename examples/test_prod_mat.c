#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

int get_nb(QuantumRegister *qreg, int n) {
    int64 size = 1ULL << n;
    for(int64 i = 0; i < size; i++) {
        if(cabs(qregister_get_amplitude(qreg, i)) > EPSILON) return i;
    }
    return -1;
}

int main() {
    srand(time(NULL));
    
    int n = 5; int N = 1 << n;
    int maxa = 5;

    int a = rand()%(maxa-1)+1;
    while(gcd(a, N) != 1 || a == 1) a = rand()%(maxa-1)+1;

    for(int i = 0; i < N; i++) {
        QuantumRegister *qreg = qregister_create(n);
        qregister_set_number(qreg, i);
        apply_prod(qreg, 0, n, a, N);
        int res = get_nb(qreg, n);
        printf("%d * %d = %d [%d]\n", a, i, res, N);
        qregister_free(qreg);
    }

    return EXIT_SUCCESS;
}