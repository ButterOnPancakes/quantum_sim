#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdio.h>

typedef FILE* graph;

graph init_graph(const char *title, const char *xlabel, const char *ylabel);
void close_graph(graph g);

void plot(graph g, double *x, double *y, int n, const char *title);
void histogram(graph g, double *x, double *y, int n, const char *title);

#endif