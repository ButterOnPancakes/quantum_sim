#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define ITERATIONS 5

int phase_estimation(QuantumRegister *eigenvector, int precision, int a, int N) {
    QuantumRegister *fst_reg = qregister_create(precision); // precision of r

    QuantumRegister *qreg = qregister_fuse(fst_reg, eigenvector); qregister_free(fst_reg);
    ClassicalRegister *creg = cregister_create(precision);

    apply_n_hadamard(qreg, 0, precision);
    printf("START PROD\n");
    for(int i = 0; i < precision; i++) {
        apply_controlled_prod_exp(qreg, precision-1-i, precision, eigenvector->nb_qbits, a, N, 1 << i);
    }
    printf("START IQFT\n");
    apply_iqft(qreg, 0, precision);

    for(int i = 0; i < precision; i++) {
        qregister_measure(qreg, i, creg, i);
    }

    int res = cregister_calc_number(creg);
    qregister_free(qreg);
    cregister_free(creg);

    return res;
}

void continued_fraction(double frac, int *quotients, int i, int n) {
    if(i <= n) {
        quotients[i] = floor(frac);
        double rest = frac - quotients[i];
        continued_fraction(1/rest, quotients, i+1, n);
    }
}

// lcm is associative
int lcm_array(int *array, int n) {
    int res = 1;
    for(int i = 0; i < n; i++) {
        res = lcm(res, array[i]);
    }
    return res;
}

int order_finding(int a, int N) {
    int n = ceil(log2(N));
    int precision = 2 * n;

    printf("%d = %d (precision) + %d (U eigenvector size) qubits used\n", n + precision, precision, n);
    int *obtained_c = malloc_custom(ITERATIONS * sizeof(int));

    printf("STARTING ORDER FINDING\n");

    for(int k = 0; k < ITERATIONS; k++) {
        printf("PHASE ESTIMATION\n");
        // 1 is the average of U eigenvectors, thus each eigenvalue have 1/r of being measured
        QuantumRegister *eigenvector = qregister_create(n);
        eigenvector->array[0] = 0; eigenvector->array[1] = 1;

        int output = phase_estimation(eigenvector, precision, a, N);
        qregister_free(eigenvector);

        double frac = (double) output / pow(2, 2*n);
        printf("OUTPUT : %d / FRACTION OBTAINED : %f\n", output, frac);

        int *quotients = malloc_custom(n * sizeof(int));
        continued_fraction(frac, quotients, 0, n);

        int pi_2 = 0, pi_1 = 1;
        int qi_2 = 1, qi_1 = 0;
        int i = 0;
        while(qi_1 < N || fabs(frac - pi_2/(double) qi_2) > (double) (2 * qi_2 * qi_2)) {
            int pi = quotients[i] * pi_1 + pi_2;
            int qi = quotients[i] * qi_1 + qi_2;

            pi_2 = pi_1; pi_1 = pi;
            qi_2 = qi_1; qi_1 = qi;

            i++;
        }
        free_custom(quotients);

        // end of the loop, qi_1 >= N et pi_2, qi_2 verifient legendre, b/c = j/r = frac
        int c = qi_2;

        obtained_c[k] = c;
        printf("C OBTAINED : %d\n", c);
    }

    int res = lcm_array(obtained_c, ITERATIONS);
    free_custom(obtained_c);

    return res;
}

int main(int argc, char *argv[]) {
    if(argc < 2) {fprintf(stderr, "./shor <N>"); return 1;}
    int N = atoi(argv[1]);

    printf("\n\n---------------- STARTING SHOR WITH N = %d ----------------\n\n", N);

    if(N%2==0) {
        printf("%d = %d * %d\n", N, N/2, 2);
        return 0;
    }

    while(true) { // Will exit with returns
        int a = rand()%(N-1)+1;
        if(gcd(a, N) != 1) {
            printf("%d = %d * %d\n", N, gcd(a, N), N/gcd(a, N));
            return 0;
        }

        //a is now a coprime with N
        int r = order_finding(a, N);
        if(r%2==1) {
            printf("R IS PAIR, RESTARTING\n\n");
            continue;
        }

        int g = gcd(fexp(a, r/2) + 1, N);
        if(g != 1 && g != N) {
            printf("%d = %d * %d\n", N, g, N/g);
            return 0;
        }

        printf("TRIVIAL G FOUND, RESTARTING\n\n");
    }

    return 1;
}