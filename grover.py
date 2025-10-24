# Built-in modules
import math
import numpy as np
import matplotlib.pyplot as plt
 
# Imports from Qiskit
from qiskit import QuantumCircuit, transpile
from qiskit.circuit.library import grover_operator, MCMTGate, ZGate, UnitaryGate

from qiskit_aer import AerSimulator
from qiskit.visualization import plot_histogram

def get_matrix(circuit : QuantumCircuit):
    circ = circuit.copy()
    circ.save_unitary()
    simulator = AerSimulator(method = 'unitary')
    circ = transpile(circ, simulator)
    result = simulator.run(circ).result()
    unitary = result.get_unitary(circ)
    print("Circuit unitary:\n", np.asarray(unitary).round(2))

def phase_oracle(marked_states):
    """Build a Grover oracle for multiple marked states
 
    Here we assume all input marked states have the same number of bits
 
    Parameters:
        marked_states (str or list): Marked states of oracle
 
    Returns:
        QuantumCircuit: Quantum circuit representing Grover oracle
    """
    if not isinstance(marked_states, list):
        marked_states = [marked_states]
    # Compute the number of qubits in circuit
    num_qubits = len(marked_states[0])
 
    qc = QuantumCircuit(num_qubits)
    # Mark each target state in the input list
    for target in marked_states:
        # Flip target bit-string to match Qiskit bit-ordering
        rev_target = target[::-1]
        # Find the indices of all the '0' elements in bit-string
        zero_inds = [
            ind
            for ind in range(num_qubits)
            if rev_target[ind] == '0'
        ]
        # Add a multi-controlled Z-gate with pre- and post-applied X-gates (open-controls)
        # where the target bit-string has a '0' entry
        if zero_inds: qc.x(zero_inds)
        qc.compose(MCMTGate(ZGate(), num_qubits - 1, 1), inplace=True)
        if zero_inds: qc.x(zero_inds)

    return qc

def diffusion_operator(num_qbits):
    num_states = 2**num_qbits
    zero_state = np.zeros(shape=(num_states, 1))
    zero_state[0][0] = 1
    diff_matrix = 2 * np.matmul(zero_state, np.transpose(zero_state)) - np.identity(num_states)
    
    diff_op = UnitaryGate(data=diff_matrix, label="Diff Op")

    diffusion = QuantumCircuit(num_qbits)
    diffusion.append(diff_op, qargs=list(np.arange(num_qbits)))

    return diffusion

def grover_operator(marked_states):
    op = phase_oracle(marked_states)
    op.h(range(op.num_qubits))
    op.compose(diffusion_operator(op.num_qubits), inplace=True)
    op.h(range(op.num_qubits))
    return op

marked_states = ["110", "111"]

circuit = phase_oracle(marked_states)

grover_op = grover_operator(marked_states)
grover_op.draw(output="mpl")

optimal_num_iterations = math.floor(
    math.pi
    / (4 * math.asin(math.sqrt(len(marked_states) / 2**grover_op.num_qubits)))
)

qc = QuantumCircuit(grover_op.num_qubits)
# Create even superposition of all basis states
qc.h(range(grover_op.num_qubits))

# Apply Grover operator the optimal number of times
qc.compose(grover_op.power(optimal_num_iterations), inplace=True)

get_matrix(qc)

# Measure all qubits
qc.measure_all()
qc.draw(output="mpl", style="iqp")

# Run on the local simulator
simulator = AerSimulator()  # Initialize the simulator

# Transpile the circuit for the simulator
compiled_circuit = transpile(qc, simulator)

# Run the transpiled circuit (e.g., for 1000 shots)
job = simulator.run(compiled_circuit, shots=1000)
result = job.result()

# Get the results
counts = result.get_counts()
print("Measurement results:", counts)

# Visualize the results
plot_histogram(counts)