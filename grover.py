from qiskit import QuantumCircuit, transpile
from qiskit.visualization import plot_histogram
from qiskit_aer import AerSimulator
import numpy as np

def grover_basic_gates(target_binary="11"):
    """
    Grover's algorithm using ONLY basic gates for clarity
    target_binary: the state we're searching for (e.g., "11" for |11⟩)
    """
    n_qubits = len(target_binary)
    qc = QuantumCircuit(n_qubits, n_qubits)
    
    print(f"Building Grover's algorithm to search for |{target_binary}⟩")
    print("=" * 50)
    
    # STEP 1: Create equal superposition
    print("Step 1: Apply Hadamard to all qubits → Equal superposition")
    for i in range(n_qubits):
        qc.h(i)
    qc.barrier()
    print("State: (|00⟩ + |01⟩ + |10⟩ + |11⟩)/2" if n_qubits == 2 else "Equal superposition")
    
    # STEP 2: Oracle - marks the target state with a phase flip
    print(f"\nStep 2: Oracle - marks |{target_binary}⟩ with negative phase")
    
    # Convert target to list of bits
    target_bits = [int(bit) for bit in target_binary]
    
    # Apply X gates to qubits where target is 0
    # This transforms our target state to |11...1⟩
    for i, bit in enumerate(target_bits):
        if bit == 0:
            qc.x(i)
    
    # Apply controlled-Z operation (marks |11⟩ state)
    if n_qubits == 1:
        qc.z(0)  # Phase flip for single qubit
    elif n_qubits == 2:
        qc.cz(0, 1)  # Controlled-Z marks |11⟩
    else:
        # For more qubits, build multi-controlled Z from basic gates
        qc.h(n_qubits-1)
        # Build multi-controlled NOT (Toffoli-like) from CNOTs
        for i in range(n_qubits-1):
            qc.cx(i, n_qubits-1)
        qc.h(n_qubits-1)
    
    # Uncompute the X gates
    for i, bit in enumerate(target_bits):
        if bit == 0:
            qc.x(i)
    
    qc.barrier()
    print("Oracle complete: |" + target_binary + "⟩ now has negative phase")
    
    # STEP 3: Diffusion operator - amplifies the marked state
    print("\nStep 3: Diffusion operator - amplifies the marked state")
    
    # Apply H gates to all qubits
    for i in range(n_qubits):
        qc.h(i)
    
    # Apply X gates to all qubits
    for i in range(n_qubits):
        qc.x(i)
    
    # Apply controlled-Z operation (similar to oracle but for all states)
    if n_qubits == 1:
        qc.z(0)
    elif n_qubits == 2:
        qc.cz(0, 1)
    else:
        qc.h(n_qubits-1)
        for i in range(n_qubits-1):
            qc.cx(i, n_qubits-1)
        qc.h(n_qubits-1)
    
    # Apply X gates to all qubits
    for i in range(n_qubits):
        qc.x(i)
    
    # Apply H gates to all qubits
    for i in range(n_qubits):
        qc.h(i)
    
    qc.barrier()
    print("Diffusion complete: Amplified probability of marked state")
    
    # STEP 4: Measurement
    print("\nStep 4: Measure all qubits")
    for i in range(n_qubits):
        qc.measure(i, i)
    
    return qc

# Let's test it for different target states
print("GROVER'S ALGORITHM - GATE BY GATE EXPLANATION")
print("=" * 60)

bits = "1011"

# Test case 1: Search for |11⟩
qc_11 = grover_basic_gates(target_binary=bits)
print(f"\nCircuit for searching |{bits}⟩:")
print(qc_11.draw())

# Run simulation
simulator = AerSimulator()
compiled_circuit = transpile(qc_11, simulator)
result = simulator.run(compiled_circuit, shots=1024).result()
counts = result.get_counts()

print(f'\nResults for {bits} search:')
print(counts)