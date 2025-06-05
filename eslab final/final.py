import matplotlib.pyplot as plt
import numpy as np

# Frequencies (MHz)
frequencies = np.array([80, 64, 60, 48, 40, 24, 20, 16, 12, 8, 4])

# Group 1 (Computing Task)
g1_energy = np.array([13, 16, 18, 22, 26, 44, 52, 64, 85, 120, 209])
g1_time = np.array([24.064262, 30.117962, 32.141396, 40.244011, 48.372345,
                    81.150734, 97.717468, 122.791084, 164.999161, 251.640533, 528.837036])

# Group 2 (I/O Task)
g2_freq = np.array([80, 60, 40, 20, 16, 12, 8, 4])
g2_energy = np.array([11, 12, 12, 13, 14, 14, 16, 19])
g2_time = np.array([1.397126, 22.275040, 64.027603, 189.344711,
                    201.610275, 211.127655, 246.845993, 323.674835])

# Group 3 (Unknown Task)
g3_freq = np.array([80, 60, 40, 20, 16, 12, 8, 4])
g3_energy = np.array([4, 5, 6, 5, 6, 7, 6, 9])
g3_time = np.array([11.478118, 67.051292, 70.823860, 82.174347,
                    89.107452, 96.676445, 117.557503, 170.211975])

# === Plot 1: Energy vs Frequency (linear) ===
plt.figure(figsize=(8, 6))
plt.plot(frequencies, g1_energy, 'o-', label='Group 1 (Computing)')
plt.plot(g2_freq, g2_energy, 's-', label='Group 2 (I/O)')
plt.plot(g3_freq, g3_energy, '^-', label='Group 3 (Unknown)')
plt.xlabel('Frequency (MHz)')
plt.ylabel('Energy (mAh)')
plt.title('Energy vs Frequency (Linear Scale)')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()

# === Plot 2: Energy vs Frequency (log-log) ===
plt.figure(figsize=(8, 6))
plt.plot(frequencies, g1_energy, 'o-', label='Group 1 (Computing)')
plt.plot(g2_freq, g2_energy, 's-', label='Group 2 (I/O)')
plt.plot(g3_freq, g3_energy, '^-', label='Group 3 (Unknown)')
plt.xlabel('Frequency (MHz)')
plt.ylabel('Energy (mAh)')
plt.title('Energy vs Frequency (Log-Log Scale)')
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which='both')
plt.legend()
plt.tight_layout()
plt.show()

# === Plot 3: Time vs Frequency (linear) ===
plt.figure(figsize=(8, 6))
plt.plot(frequencies, g1_time, 'o-', label='Group 1 (Computing)')
plt.plot(g2_freq, g2_time, 's-', label='Group 2 (I/O)')
plt.plot(g3_freq, g3_time, '^-', label='Group 3 (Unknown)')
plt.axhline(y=185, color='r', linestyle='--', label='Time = 185 sec')
plt.xlabel('Frequency (MHz)')
plt.ylabel('Execution Time (sec)')
plt.title('Execution Time vs Frequency (Linear Scale)')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()

# === Plot 4: Time vs Frequency (log-log) ===
plt.figure(figsize=(8, 6))
plt.plot(frequencies, g1_time, 'o-', label='Group 1 (Computing)')
plt.plot(g2_freq, g2_time, 's-', label='Group 2 (I/O)')
plt.plot(g3_freq, g3_time, '^-', label='Group 3 (Unknown)')
plt.axhline(y=185, color='r', linestyle='--', label='Time = 185 sec')
plt.xlabel('Frequency (MHz)')
plt.ylabel('Execution Time (sec)')
plt.title('Execution Time vs Frequency (Log-Log Scale)')
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which='both')
plt.legend()
plt.tight_layout()
plt.show()