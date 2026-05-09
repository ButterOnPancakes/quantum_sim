#include "../../builder/registers.h"
#include "../../simulator/operations.h"

#include "../../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

void run_phase_estimation(FILE *graph, int precision, double theta) {
    QuantumRegister *out_reg = qregister_create(precision); // precision of r
    qregister_set_number(out_reg, 0);
    QuantumRegister *eigenvector = qregister_create(1);
    qregister_set_number(eigenvector, 1);
    
    QuantumRegister *qreg = qregister_fuse(out_reg, eigenvector); qregister_free(out_reg); qregister_free(eigenvector);

    for(int i = 0; i < precision; i++) apply_gate_hadamard(qreg, i);
    for(int i = 0; i < precision; i++) apply_controlled_rotation(qreg, i, precision, 2 * M_PI * theta * (1 << i));
    apply_iqft(qreg, 0, precision);

    double proba_tot = 0;
    int64 size = 1ULL << (precision + 1);
    for(int64 i = 1 << precision; i < size; i++) {
        double proba = qregister_calc_proba_multiple(qreg, i);
        proba_tot += proba;
        fprintf(graph, "%lf %lf\n", i / (double) (1 << precision) - 1., proba);
    }
    assert(proba_tot > 1. - EPSILON);
    fprintf(graph, "e\n");

    qregister_free(qreg);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    FILE *graph = popen("gnuplot", "w");
    if(graph == NULL) printf("WTH\n");

    double theta = 0.4;
    if(argc >= 2) {
        theta = atof(argv[1]);
    }

    int minPrec = 3, maxPrec = 10;

    fprintf(graph, "set terminal png\n");
    fprintf(graph, "set output 'logs/phase_estimation_precision.png'\n");
    fprintf(graph, "set title 'Number of precision qubit impact'\n");

    fprintf(graph, "set xlabel 'Decimal Number obtainable'\n"); fprintf(graph, "set xrange [0:1]\n");
    fprintf(graph, "set ylabel 'Probability of obtaining it'\n"); fprintf(graph, "set yrange [0:1]\n");

    fprintf(graph, "set style data histograms\n"); fprintf(graph, "set style fill solid 0.5\n");
    
    fprintf(graph, "plot '-' with boxes title '%d qubits'", minPrec);
    for(int i = 0; i < maxPrec - minPrec; i++) {
        fprintf(graph, ", '-' with boxes title '%d qubits'", minPrec + i + 1);
    }
    fprintf(graph, "\n");

    for(int precision = minPrec; precision <= maxPrec; precision++) {
        run_phase_estimation(graph, precision, theta);
    }

    pclose(graph);
    return EXIT_SUCCESS;
}