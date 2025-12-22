import numpy as np

def apply_unitary(state : np.ndarray, U : np.ndarray, targets : list, n_qubits : int) -> np.ndarray:
    """
    state: 1D array of size 2^n
    U: 2^k x 2^k unitary
    targets: list of k qubit indices
    n_qubits: total qubits
    """
    k = len(targets)
    state_tensor = state.reshape([2] * n_qubits)

    remaining = [i for i in range(n_qubits) if i not in targets]
    perm = targets + remaining
    inv_perm = np.argsort(perm)

    state_perm = state_tensor.transpose(perm)
    state_flat = state_perm.reshape(2**k, 2**(n_qubits - k))

    state_flat = U @ state_flat

    state_perm = state_flat.reshape([2] * n_qubits)
    state_tensor = state_perm.transpose(inv_perm)

    return state_tensor.reshape(-1)

state = np.array([1, 0, 0, 0, 0, 0, 0, 0], dtype=complex)  # |000>
U = np.array([[0, 1], [1, 0]], dtype=complex)  # X gate
CNOT = np.array([[1, 0, 0, 0],
                 [0, 1, 0, 0],
                 [0, 0, 0, 1],
                 [0, 0, 1, 0]], dtype=complex)  # CNOT gate
new_state = apply_unitary(state, CNOT, [0, 1], 3)
print("New state:\n", new_state.round(2))