#include "gnuplot.h"

#include <stdio.h>
#include <stdlib.h>

graph init_graph(const char *title, const char *xlabel, const char *ylabel) {
    graph g = popen("gnuplot -persistent", "w");
    if (g == NULL) {
        perror("popen");
        return NULL;
    }
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "logs/%s.png", title);

    fprintf(g, "set terminal png\n");
    fprintf(g, "set output '%s'\n", buffer);
    fprintf(g, "set title '%s'\n", title);
    fprintf(g, "set xlabel '%s'\n", xlabel);
    fprintf(g, "set ylabel '%s'\n", ylabel);
    fprintf(g, "set grid\n");
    return g;
}
void close_graph(graph g) {
    if (g != NULL) {
        pclose(g);
    }
}

void plot(graph g, double *x, double *y, int n, const char *title) {
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
void histogram(graph g, double *x, double *y, int n, const char *title) {
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