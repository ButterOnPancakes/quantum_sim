#ifndef REGISTERS_INTERNAL_H
#define REGISTERS_INTERNAL_H

#include <complex.h>
#include <stdint.h>
#include <stdbool.h>
#include "../utils/utils.h"

struct qregister_s {
    double complex* array;
    int64 size;
    int nb_qbits;
};

struct cregister_s {
    bool* array;
    int size;
};

#endif // REGISTERS_INTERNAL_H
