#ifndef EMMS_SIM_H
#define EMMS_SIM_H

#include "../builder/emms.h"
#include <complex.h>
#include <stdbool.h>

/**
 * BlockView: A mathematical view into a statevector.
 * Represents a vector V where V[k] = data[k * stride].
 * This abstraction allows us to perform operations on subspaces (qubits)
 * without physical memory reordering.
 */
typedef struct {
    double complex *data;
    uint64_t stride;
} BlockView;

/**
 * Computes the statevector output of the circuit assuming the initial state is |0...0>.
 */
double complex *emms_compute_statevector(Node *circuit);

/**
 * Applies a quantum operator (Node) to a statevector view.
 * @param node The operator (Matrix, Sum, Product, or Tensor)
 * @param in   Input statevector view
 * @param out  Output statevector view
 * @param aux  Scratchpad for intermediate results
 * @param acc  Accumulation flag (out += Op * in if true, else out = Op * in)
 */
void emms_apply_inplace(Node *node, 
                        BlockView in, 
                        BlockView out, 
                        BlockView aux, 
                        bool acc);

#endif
