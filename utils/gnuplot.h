#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdio.h>

typedef FILE* graph;

graph graph_create(const char *title, const char *xlabel, const char *ylabel);
void graph_free(graph g);

void graph_plot(graph g, double *x, double *y, int n, const char *title);
void graph_histogram(graph g, double *x, double *y, int n, const char *title);

#endif