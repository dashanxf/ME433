import csv
import numpy as np
from matplotlib import pyplot as plt

t = [] # column 0
data1 = [] # column 1

with open('sigB.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        data1.append(float(row[1])) # second column
        
t = np.array(t)
y = np.array(data1)

# === FFT ===
n = len(y)
dt = np.mean(np.diff(t))          # sampling interval
fs = 1 / dt                       # sampling frequency

Y = np.fft.fft(y) / n
freq = np.fft.fftfreq(n, d=dt)

# Take only positive frequencies
mask = freq >= 0
freq = freq[mask]
Y = np.abs(Y[mask])

# === Plot ===
plt.figure()

# --- Subplot 1: Time domain ---
plt.subplot(2, 1, 1)
plt.plot(t, y)
plt.xlabel('Time')
plt.ylabel('Amplitude')
plt.title('Signal (Time Domain)')
plt.grid(True)

# --- Subplot 2: Frequency domain ---
plt.subplot(2, 1, 2)
plt.plot(freq, Y)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude')
plt.title('FFT')
plt.grid(True)

plt.tight_layout()
plt.show()


# -------- Moving Average Filter --------
X = 250

y_filt = []

for i in range(n):
    if i < X:
        avg = np.mean(y[:i+1])
    else:
        avg = np.mean(y[i-X+1:i+1])
    y_filt.append(avg)

y_filt = np.array(y_filt)

# -------- Plot Time Domain --------
plt.figure()
plt.subplot(2, 1, 1)
plt.plot(t, y, 'k', label='Unfiltered')
plt.plot(t, y_filt, 'r', label='Filtered')
plt.title(f'Moving Average Filter (X={X})')
plt.legend()
plt.xlabel('Time')
plt.ylabel('Amplitude')

# -------- FFT --------
Y = np.fft.fft(y) / n
Y = Y[range(int(n/2))]
Y_filt = np.fft.fft(y_filt) / n
Y_filt = Y_filt[range(int(n/2))]
# Only plot first half (real signal symmetry)
freqs = np.arange(n/2)

plt.subplot(2, 1, 2)
plt.plot(freqs, np.abs(Y), 'k', label='Unfiltered FFT')
plt.plot(freqs, np.abs(Y_filt), 'r', label='Filtered FFT')
plt.title(f'FFT Comparison (X={X})')
plt.legend()
plt.xlabel('Frequency Bin')
plt.ylabel('Magnitude')
plt.tight_layout()
plt.show()


# -------- IIR Filter --------
A = 0.98
B = 0.02 

y_filt = []

for i in range(n):
    if i == 0:
        y_filt.append(y[0])  # initialize
    else:
        val = A * y_filt[i-1] + B * y[i]
        y_filt.append(val)

y_filt = np.array(y_filt)

# -------- Time Domain Plot --------
plt.figure()
plt.subplot(2, 1, 1)
plt.plot(t, y, 'k', label='Unfiltered')
plt.plot(t, y_filt, 'r', label='IIR Filtered')
plt.title(f'IIR Low-Pass Filter (A={A}, B={B})')
plt.xlabel('Time')
plt.ylabel('Amplitude')
plt.legend()

# -------- FFT --------
Y = np.fft.fft(y) / n
Y = Y[range(int(n/2))]
Y_filt = np.fft.fft(y_filt) / n
Y_filt = Y_filt[range(int(n/2))]

freqs = np.arange(n/2)

plt.subplot(2, 1, 2)
plt.plot(freqs, np.abs(Y), 'k', label='Unfiltered FFT')
plt.plot(freqs, np.abs(Y_filt), 'r', label='Filtered FFT')
plt.title(f'FFT Comparison (A={A}, B={B})')
plt.xlabel('Frequency Bin')
plt.ylabel('Magnitude')
plt.tight_layout()
plt.legend()
plt.show()