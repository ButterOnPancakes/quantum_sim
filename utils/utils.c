#include "utils.h"

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