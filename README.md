# ⚛️ quantum_sim

A quantum computer simulator written in C. It simulates quantum circuits by operating directly on a statevector, supporting standard gates, controlled gates, custom unitary matrices, and mid-circuit measurements.

---

## 🌿 Repository Structure (Branches)

| Branch | Description |
|---|---|
| `main` | **Primary simulator.** Applies gates directly to the statevector one at a time — the recommended, memory-efficient approach. |
| `naive` | Computes the full circuit matrix (Kronecker product of all gates) and applies it at once. Straightforward but exponential in memory. |
| `emms` | Implements a complex data structure to reduce memory pressure during circuit simulation. |
| `optimised` | Optimised statevector simulation: avoids computing any circuit-level matrix by applying each gate in-place, maximising cache performance. |

> **Tip:** Start with `main`. Switch to `optimised` if you need better performance, or `naive` if you need a reference implementation.

---

## ✨ Features

- **Statevector simulation** of arbitrary quantum circuits
- **Standard single-qubit gates**: Hadamard (H), Pauli X/Y/Z, Phase (P)
- **Controlled gates**: Controlled-U for any single-qubit gate
- **Custom multi-qubit gates**: supply any 2ᵏ × 2ᵏ unitary matrix
- **Mid-circuit measurement** with Born-rule collapse and renormalisation
- **Register fusion** (`qregister_fuse`) to compose multi-qubit systems
- **Classical register** to store measurement outcomes
- **Execution timing** built-in to `circuit_execute`
- **Structured logging** of circuit execution to a log file
- **Statevector visualisation** via `gnuplot`
- **OpenMP parallelism** for gate application
- **Example algorithms**: QFT, Grover's search, Shor's factoring, Quantum Teleportation

---

## 📁 Project Structure

```
quantum_sim/
├── Makefile
├── builder/            # Circuit and register construction API
│   ├── circuit.c/h     # QuantumCircuit — create, populate, print, free
│   ├── register.c/h    # QuantumRegister and ClassicalRegister (opaque types)
│   ├── gaterep.c/h     # Gate representation types and constructors
│   └── internal.h      # Internal struct definitions (not for end users)
│
├── simulator/          # Quantum gate application & execution engine
│   ├── gates.c/h       # In-place gate kernels (single, controlled, custom, measure)
│   ├── opti_sim.c/h    # circuit_execute() — the main simulation entry point
│   └── ...
│
├── utils/              # Utility and support modules
│   ├── list.c/h        # Generic singly-linked list with iterator
│   ├── utils.c/h       # malloc/calloc/free wrappers, timing helpers
│   ├── logger.c/h      # Structured file logger
│   └── gnuplot.c/h     # Statevector and histogram plotting via gnuplot
│
├── examples/           # Runnable quantum algorithm demonstrations
│   ├── qft.c           # Quantum Fourier Transform
│   ├── grover.c        # Grover's search algorithm
│   ├── shor.c          # Shor's integer factoring algorithm
│   └── teleportation.c # Quantum teleportation protocol
│
├── bin/                # Build output (created by make)
├── gui/                # (Optional) GUI front-end using raylib
└── logs/               # Runtime log output
```

---

## 🔧 Dependencies

| Dependency | Purpose | Install |
|---|---|---|
| `gnuplot` | Statevector plotting | `sudo apt install gnuplot` |
| `raylib` *(optional)* | GUI (`-lraylib`) | `sudo apt install libraylib-dev` |

---

## 🏗️ Build Instructions

```bash
# Clone the repository
git clone https://github.com/ButterOnPancakes/quantum_sim.git
cd quantum_sim

# Build all examples
make all
```

Compiled binaries are placed under `bin/examples/`.

---

## 🚀 Running the Examples

### Quantum Fourier Transform

```bash
./bin/examples/qft <number_of_qubits>
# Example: 4-qubit QFT
./bin/examples/qft 4
```

Prints execution time and plots the output statevector.

### Grover's Search Algorithm

```bash
./bin/examples/grover
```

Runs Grover's algorithm on a 4-qubit register searching for the marked state `|1⟩`. Prints the measurement result and plots the probability distribution.

### Shor's Factoring Algorithm

```bash
./bin/examples/shor
# When prompted, enter a number to factor:
# Enter number to factor (N): 15
```

Runs the full quantum phase estimation + continued fractions post-processing pipeline.

### Quantum Teleportation

```bash
./bin/examples/teleportation
```

Demonstrates the three-qubit teleportation protocol using Bell state preparation, Alice's measurement, and Bob's correction gates.

---

## 🔌 Creating a Custom Circuit

Here is a minimal example that creates a Bell state (|Φ⁺⟩ = (|00⟩ + |11⟩) / √2):

```c
#include "builder/circuit.h"
#include "builder/register.h"
#include "simulator/opti_sim.h"

int main() {
    // 1. Allocate registers
    QuantumRegister  *qreg = qregister_create(2);
    ClassicalRegister *creg = cregister_create(2);

    // 2. Build the circuit
    QuantumCircuit *qc = circuit_create(2);
    add_unitary_gate(qc, 0, GATE_H, 0.0);          // H on qubit 0
    add_control_gate(qc, 0, 1, GATE_X, 0.0);        // CNOT: control=0, target=1
    add_measure(qc, 0, 0);                           // Measure qubit 0 → cbit 0
    add_measure(qc, 1, 1);                           // Measure qubit 1 → cbit 1

    // 3. Execute
    circuit_execute(qc, qreg, creg, false);

    // 4. Read results
    qregister_print(stdout, qreg);
    cregister_print(stdout, creg);

    // 5. Free resources
    circuit_free(qc);
    qregister_free(qreg);
    cregister_free(creg);
    return 0;
}
```

### Adding a Custom Unitary Gate

Provide any 2ᵏ × 2ᵏ row-major matrix and the list of target qubit indices:

```c
// SWAP gate (4x4 matrix, 2 qubits)
double complex SWAP[16] = {
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};
int targets[2] = {0, 1};
add_custom_gate(qc, 2, targets, SWAP, "SWAP");
```

---

## 📖 API Reference

### Quantum Register (`builder/register.h`)

```c
// Create an n-qubit register initialised to |0...0⟩
QuantumRegister *qregister_create(int nqubits);

// Fuse two registers into one (tensor product)
QuantumRegister *qregister_fuse(QuantumRegister *q1, QuantumRegister *q2);

// Access the statevector (array of 2^n complex amplitudes)
double complex *qregister_get_statevector(const QuantumRegister *qregister);
int             qregister_get_num_qubits(const QuantumRegister *qregister);

void qregister_print(FILE *channel, QuantumRegister *qregister);
void qregister_free(QuantumRegister *qregister);
```

### Classical Register (`builder/register.h`)

```c
ClassicalRegister *cregister_create(int nbits);
int  cregister_get_bit(const ClassicalRegister *creg, int index);
int  cregister_get_num_bits(const ClassicalRegister *creg);
void cregister_print(FILE *channel, ClassicalRegister *creg);
void cregister_free(ClassicalRegister *creg);
```

### Circuit Builder (`builder/circuit.h`)

```c
QuantumCircuit *circuit_create(int nb_qbits);
void circuit_free(QuantumCircuit *circuit);
void circuit_print(FILE *channel, QuantumCircuit *circuit);

// Single-qubit gate (GATE_H, GATE_X, GATE_Y, GATE_Z, GATE_PHASE)
void add_unitary_gate(QuantumCircuit *circuit, int target, SingleBitGate gate, double phase);

// Controlled-U gate
void add_control_gate(QuantumCircuit *circuit, int control, int target, SingleBitGate gate, double phase);

// Arbitrary k-qubit gate  (mat must be 2^k × 2^k row-major)
void add_custom_gate(QuantumCircuit *circuit, int nb_qbits, int *targets, double complex *mat, char *label);

// Measurement: collapses qubit `qbit`, result stored in classical bit `cbit`
void add_measure(QuantumCircuit *circuit, int qbit, int cbit);
```

### Simulator (`simulator/opti_sim.h`)

```c
// Execute the circuit; returns wall-clock execution time in seconds.
// Pass log=true to write a full execution trace to `circuit_execution.log`.
double circuit_execute(QuantumCircuit *circuit,
                       QuantumRegister *qregister,
                       ClassicalRegister *cregister,   // may be NULL if no measurements
                       bool log);
```

### Gate Types (`builder/gaterep.h`)

```c
typedef enum {
    GATE_I,      // Identity
    GATE_H,      // Hadamard
    GATE_X,      // Pauli-X (NOT)
    GATE_Y,      // Pauli-Y
    GATE_Z,      // Pauli-Z
    GATE_PHASE   // Phase gate: diag(1, e^{i·phase})
} SingleBitGate;
```

### Visualisation (`utils/gnuplot.h`)

```c
graph graph_create(const char *title, const char *xlabel, const char *ylabel);
void  graph_statevector(graph g, double complex *statevector, int n);
void  graph_histogram(graph g, double *x, double *y, int n, const char *label);
void  graph_free(graph g);
```

---

## 📝 Notes

- Qubit indices use **LSB = 0** convention.
- The statevector has **2ⁿ** complex amplitudes for an *n*-qubit system. Memory usage scales exponentially; simulating beyond ~25–28 qubits will exhaust typical RAM.
- `circuit_execute` accepts `cregister = NULL` when no measurements are needed (e.g., pure unitary evolution).
- All memory allocation is routed through `malloc_custom`/`free_custom` wrappers (see `utils/utils.h`) for easier leak tracking.
