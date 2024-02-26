#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np

T_MIN = 0.800
T_MAX = 0.900
tau = 0.120

x = np.linspace(0, T_MAX, 100)
y_none = [(t - T_MIN) / (T_MAX - T_MIN) for t in x]
y_linear = [1 + (t - T_MAX) / tau for t in x]
y_exp = [np.exp((t - T_MAX) / tau) for t in x]

plt.plot(x, y_none, label='None')
plt.plot(x, y_linear, label='Linear')
plt.plot(x, y_exp, label='Exponential')

plt.xlabel('time (sec)')
plt.ylabel('normalized value between 0 and 1')
plt.xlim(T_MIN, T_MAX)
plt.ylim(0, 1)
plt.legend()
plt.grid(True)
plt.show()
