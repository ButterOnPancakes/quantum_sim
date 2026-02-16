#include "gnuplot.h"

#include <stdio.h>
#include <stdlib.h>

graph graph_create(const char *title, const char *xlabel, const char *ylabel) {
    graph g = popen("gnuplot -persistent", "w");
    if (g == NULL) {
        perror("popen");
        return NULL;
    }
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "naive_version/logs/%s.png", title);

    fprintf(g, "set terminal png\n");
    fprintf(g, "set output '%s'\n", buffer);
    fprintf(g, "set title '%s'\n", title);
    fprintf(g, "set xlabel '%s'\n", xlabel);
    fprintf(g, "set ylabel '%s'\n", ylabel);
    fprintf(g, "set grid\n");
    return g;
}
void graph_free(graph g) {
    if (g != NULL) {
        pclose(g);
    }
}

void graph_plot(graph g, double *x, double *y, int n, const char *title) {
    if (g == NULL) {
        return;
    }

    fprintf(g, "plot '-' with linespoints title '%s'\n", title);
    for (int i = 0; i < n; i++) {
        fprintf(g, "%lf %lf\n", x[i], y[i]);
    }
    fprintf(g, "e\n");
    fflush(g);
}
void graph_histogram(graph g, double *x, double *y, int n, const char *title) {
    if (g == NULL) {
        return;
    }
    
    fprintf(g, "set style data histograms\n");
    fprintf(g, "set style fill solid 0.5\n");
    fprintf(g, "plot '-' with boxes title '%s'\n", title);
    for (int i = 0; i < n; i++) {
        fprintf(g, "%lf %lf\n", x[i], y[i]);
    }
    fprintf(g, "e\n");
    fflush(g);
}
void graph_statevector(graph g, double complex *statevector, int n) {
    if (g == NULL) {
        return;
    }

    fprintf(g, "unset key\n");
    fprintf(g, "set style fill solid 0.7 border -1\n");
    fprintf(g, "set boxwidth 0.8\n");
    fprintf(g, "set grid ytics\n");
    fprintf(g, "set xrange [-0.5:%d-0.5]\n", n);
    fprintf(g, "set yrange [0:1]\n");

    /* X ticks at integer positions */
    fprintf(g, "set xtics (");
    for (int i = 0; i < n; i++) {
        fprintf(g, "\"|%02b>\" %d%s", i, i, (i < n-1) ? ", " : "");
    }
    fprintf(g, ")\n");
    
    fprintf(g, "plot '-' using 1:2 with boxes title '%s'\n", "Qbit Measurement Probabilities");


    for (int i = 0; i < n; i++) {
        fprintf(g, "%lf %lf\n", (double)i, cabs(statevector[i]) * cabs(statevector[i]));
    }

    fprintf(g, "e\n");
    fflush(g);
}
