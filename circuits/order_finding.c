#include "order_finding.h"

#include "../builder/registers.h"
#include "../simulator/operations.h"

#include "../utils/utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

int64 phase_estimation(QuantumRegister *eigenvector, int precision, int64 a, int64 N) {
    QuantumRegister *out_reg = qregister_create(precision); // precision of r
    if (!out_reg) {
        fprintf(stderr, "Error allocating memory for fst_reg\n");
        return 1;
    }

    QuantumRegister *qreg = qregister_fuse(out_reg, eigenvector); qregister_free(out_reg);
    if (!qreg) {
        fprintf(stderr, "Error allocating memory for qreg\n");
        return 1;
    }

    ClassicalRegister *creg = cregister_create(precision);

    apply_n_hadamard(qreg, 0, precision);
    for(int i = 0; i < precision; i++) {
        // controlled custom gate 
        apply_controlled_prod_exp(qreg, i, precision, eigenvector->nb_qbits, a, N, 1ULL << i);
    }
    apply_iqft(qreg, 0, precision);

    for(int i = 0; i < precision; i++) {
        qregister_measure(qreg, i, creg, i);
    }

    int64 res = cregister_calc_number(creg);
    qregister_free(qreg);
    cregister_free(creg);

    return res;
}

// lcm is associative
int64 lcm_array(int64 *array, int n) {
    int64 res = 1;
    for(int i = 0; i < n; i++) {
        if (array[i] > 0) {
            res = lcm(res, array[i]);
        }
    }
    return res;
}

int64 continued_fraction(double frac, int64 *quotients, int i, int n) {
    if(i < n) {
        quotients[i] = (int64)floor(frac);
        double rest = frac - quotients[i];
        //printf("Rest : %f\n", rest);
        if(rest < EPSILON) return i + 1;
        return continued_fraction(1/rest, quotients, i+1, n);
    }
    return n;
}

int64 order_finding(int64 a, int64 N, int ITERATIONS, Logger *logger) {
    int n = ceil(log2(N));
    int precision = 2 * n;

    char buffer[512];

    sprintf(buffer, "%d = %d (precision) + %d (U eigenvector size) qubits used\n", n + precision, precision, n);
    logger_message(logger, "ORDER FINDING", buffer);
    
    int64 *obtained_c = malloc_custom(ITERATIONS * sizeof(int64));
    if (!obtained_c) {
        fprintf(stderr, "Error allocating memory for obtained_c\n");
        return 1;
    }

    #pragma omp parallel for
    for(int k = 0; k < ITERATIONS; k++) {
        logger_message(logger, "ORDER FINDING", "PHASE ESTIMATION\n");
        
        // 1 is the average of U eigenvectors, thus each eigenvalue have 1/r of being measured
        QuantumRegister *eigenvector = qregister_create(n);
        if (!eigenvector) {
            fprintf(stderr, "Error allocating memory for eigenvector\n");
            exit(1);
        }
        
        eigenvector->array[0] = 0; eigenvector->array[1] = 1;

        int64 output = phase_estimation(eigenvector, precision, a, N);
        qregister_free(eigenvector);

        double frac = (double) output / pow(2, 2*n);

        sprintf(buffer, "OUTPUT : %ld\n", output);
        logger_message(logger, "ORDER FINDING", buffer);

        int64 *quotients = malloc_custom(precision * sizeof(int64));
        int len = continued_fraction(frac, quotients, 0, precision);

        int64 pi_2 = 0, pi_1 = 1;
        int64 qi_2 = 1, qi_1 = 0;
        int i = 0;
        while(i < len) {
            int64 pi = quotients[i] * pi_1 + pi_2;
            int64 qi = quotients[i] * qi_1 + qi_2;

            if (qi >= N) break;

            pi_2 = pi_1; pi_1 = pi;
            qi_2 = qi_1; qi_1 = qi;

            i++;
        }
        int64 b = pi_1, c = qi_1;
        free_custom(quotients);

        sprintf(buffer, "CFA OBTAINED : (%ld, %ld) %ld / %ld = %f\n", pi_1, qi_1, b, c, frac);
        logger_message(logger, "ORDER FINDING", buffer);

        obtained_c[k] = c;
    }

    int64 res = lcm_array(obtained_c, ITERATIONS);
    free_custom(obtained_c);

    return res;
}
