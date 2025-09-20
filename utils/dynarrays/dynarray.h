#ifndef DYNARRAY_H
#define DYNARRAY_H

typedef enum {
    INT_TYPE,
    DOUBLE_TYPE,
    POINTER_TYPE
} DataType;

typedef union {
    int int_val;
    double double_val;
    void *pointer_val;
} DataValue;

typedef struct dynarray DynArray;

DynArray* dynarray_create(DataType type, int initial_capacity);
/* WARNING : DOES NOT FREE POINTERS !!! */
void dynarray_free(DynArray* arr);

int dynarray_len(DynArray* arr);
DataValue dynarray_get(DynArray* arr, int index);

void dynarray_add_int(DynArray* arr, int value);
void dynarray_add_double(DynArray* arr, double value);
void dynarray_add_pointer(DynArray* arr, void *value);

#endif