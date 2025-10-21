#include "utils.h"

#include <stdio.h>

int reverse_power(int N) {
    int i = 0;
    while(N > 0) {
        N = N >> 1;
        i++;
    }
    return i - 1;
}

int fast_exp_i(int x, int n) {
    if(n == 0) return 1;
    else if (n == 1) return x;
    else if (n % 2 == 0) {
        int res = fast_exp_i(x, n/2);
        return res * res;
    }
    else {
        int res = fast_exp_i(x, (n-1)/2);
        return res * res * x;
    }
}

double fast_exp_d(double x, int n) {
    if(n == 0) return 1;
    else if (n == 1) return x;
    else if (n % 2 == 0) {
        double res = fast_exp_d(x, n/2);
        return res * res;
    }
    else {
        double res = fast_exp_d(x, (n-1)/2);
        return res * res * x;
    }
}

void print_list(int *bits, int n) {
    printf("[");
    for(int i = 0; i < n; i++) {
        if (i < n - 1) printf("%d, ", bits[i]);
        else printf("%d]\n", bits[i]);
    }
}