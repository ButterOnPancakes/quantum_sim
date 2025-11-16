#ifndef UTILS_H
#define UTILS_H

#define M_PI       3.14159265358979323846

double now_seconds();
int reverse_power(int N);

int fast_exp_i(int x, int n);
double fast_exp_d(double x, int n);

void print_list(int *bits, int n);

#endif