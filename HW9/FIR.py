import csv
import numpy as np
import matplotlib.pyplot as plt

# =========================
# Load CSV
# =========================
t = []
y = []

with open('sigD.csv') as f:
    reader = csv.reader(f)
    for row in reader:
        try:
            t.append(float(row[0]))
            y.append(float(row[1]))
        except:
            continue

t = np.array(t)
y = np.array(y)

# Remove DC (helps FFT clarity)
y = y - np.mean(y)

# =========================
# Sampling info
# =========================
dt = np.mean(np.diff(t))
print(dt)
fs = 1 / dt
print(fs)
# =========================
# FIR Filter Design (Low-pass sinc)
# =========================
num_taps = 80              # number of weights (filter length)
bandwidth_hz = 4 * fs / num_taps
cutoff_hz = 30               
cutoff = cutoff_hz / (fs/2) # normalized cutoff (Nyquist)

# Ideal sinc filter
n = np.arange(num_taps)
h = np.sinc(2 * cutoff * (n - (num_taps-1)/2))

# Apply window
window = np.hamming(num_taps)
h = h * window

# Normalize gain
h = h / np.sum(h)

# =========================
# Apply filter
# =========================
y_filt = np.convolve(y, h, mode='same')

# =========================
# FFT (before & after)
# =========================
def compute_fft(signal):
    N = len(signal)
    Y = np.fft.fft(signal) / N
    freq = np.fft.fftfreq(N, d=dt)
    mask = freq >= 0
    return freq[mask], np.abs(Y[mask])

freq, Y_raw = compute_fft(y)
_, Y_filt = compute_fft(y_filt)

# =========================
# Plot
# =========================
plt.figure(figsize=(10,8))

# --- Time domain ---
plt.subplot(2,1,1)
plt.plot(t, y, 'k', label='Raw')
plt.plot(t, y_filt, 'r', label='Filtered')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')
plt.title(f'FIR Low-pass | Bandwidth={bandwidth_hz}, cutoff={cutoff_hz} Hz, window=Hamming')
plt.legend()
plt.grid(True)

# --- Frequency domain ---
plt.subplot(2,1,2)
plt.plot(freq, Y_raw, 'k', label='Raw FFT')
plt.plot(freq, Y_filt, 'r', label='Filtered FFT')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()