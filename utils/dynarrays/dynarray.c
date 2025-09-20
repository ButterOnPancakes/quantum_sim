#include "dynarray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dynarray {
    DataType type;
    DataValue* data;
    int size;
    int capacity;
};

void dynarray_resize(DynArray* arr) {
    if (arr->size >= arr->capacity) {
        arr->capacity = (arr->capacity == 0) ? 1 : arr->capacity * 2;
        DataValue* new_data = realloc(arr->data, arr->capacity * sizeof(DataValue));
        if (!new_data) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        arr->data = new_data;
    }
}

DynArray* dynarray_create(DataType type, int initial_capacity) {
    DynArray* arr = malloc(sizeof(DynArray));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    arr->type = type;
    arr->size = 0;
    arr->capacity = initial_capacity > 0 ? initial_capacity : 1;
    arr->data = malloc(arr->capacity * sizeof(DataValue));
    
    if (!arr->data) {
        free(arr);
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    return arr;
}
void dynarray_free(DynArray* arr) {
    if (arr) {
        free(arr->data);
        free(arr);
    }
}

int dynarray_len(DynArray* arr) {
    return arr->size;
}
DataValue dynarray_get(DynArray* arr, int index) {
    if (index < 0 || index >= arr->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    return arr->data[index];
}

void dynarray_add_int(DynArray* arr, int value) {
    if (arr->type != INT_TYPE) {
        fprintf(stderr, "Type mismatch: Array is not of type INT\n");
        return;
    }
    dynarray_resize(arr);
    arr->data[arr->size++].int_val = value;
}
void dynarray_add_double(DynArray* arr, double value) {
    if (arr->type != DOUBLE_TYPE) {
        fprintf(stderr, "Type mismatch: Array is not of type DOUBLE\n");
        return;
    }
    dynarray_resize(arr);
    arr->data[arr->size++].double_val = value;
}
void dynarray_add_pointer(DynArray* arr, void* value) {
    if (arr->type != POINTER_TYPE) {
        fprintf(stderr, "Type mismatch: Array is not of type POINTER\n");
        return;
    }
    dynarray_resize(arr);
    arr->data[arr->size++].pointer_val = value;
}
