#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdio.h>
#include <complex.h>

typedef FILE* graph;

graph graph_create(const char *title, const char *xlabel, const char *ylabel);
void graph_free(graph g);

void graph_plot(graph g, double *x, double *y, int n, const char *title);
void graph_plot_comparison(graph g, double *x, double *y1, double *y2, int n, const char *title1, const char *title2);
void graph_histogram(graph g, double *x, double *y, int n, const char *title);
void graph_statevector(graph g, double complex *statevector, int n);

#endif