#ifndef GATES_H
#define GATES_H

#include "../builder/circuit.h"

#include <complex.h>

void apply_corresponding_gate(double complex g[4], SingleBitGate gt);

/* -------- common gates (2x2) -------- */
void gate_h(double complex g[4]);
void gate_x(double complex g[4]);
void gate_y(double complex g[4]);
void gate_z(double complex g[4]);

/* -------- single-qubit gate (in-place) --------
   Gate g : 2x2 row-major [g00,g01,g10,g11]
   target t: qubit index (LSB = 0)
   Complexity: O(2^n)
*/
void apply_single_qubit_inplace(double complex *state, int nqubits, int t, double complex g[4]);

/* -------- two-qubit gate (in-place) --------
   Gate G : 4x4 row-major G[row*4+col]
   q0, q1: qubit indices (distinct), q0 < q1.
   Updates 4 amplitudes at a time.
*/
void apply_two_qubit_inplace(double complex *state, int nqubits, int q0, int q1, double complex G[16]);

/* -------- controlled-U gate (generic) --------
   Control = c, Target = t
   Gate U : 2x2 row-major [u00,u01,u10,u11]
   When control bit = 1, apply U to target; leave amplitudes unchanged when control=0.
*/
void apply_controlled_u_inplace(double complex *state, int nqubits, int c, int t, double complex U[4]);

/* -------- custom multi-qubit gate (in-place) --------
   Gate U : 2^k x 2^k row-major matrix
   targets: array of k target qubit indices
   k: number of target qubits
   Applies U to the specified target qubits.
*/
void apply_custom_inplace(double complex *state, int nqbits, int *targets, int k, double complex *U);

/* -------- measurement (single qubit) --------
   Collapses state and returns measurement result (0/1).
   Uses Born rule and renormalizes remaining amplitudes.
*/
int measure_qubit_inplace(double complex *state, int nqubits, int t);

#endif