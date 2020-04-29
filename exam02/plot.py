import matplotlib.pyplot as plt
import numpy as np
import serial
import time

duration = 10

t = np.arange(0, duration,0.1)
x = []
y = []
z = []
tilt = []

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev,115200)
for i in range(0, duration * 10):
    line = str(s.readline())
    arr = line.split("[")[1].split("]")[0].split(',')
    x.append(float(arr[0]))
    y.append(float(arr[1]))
    z.append(float(arr[2]))
    tilt.append(int(arr[3]))

fig, ax = plt.subplots(2, 1)
ax[0].plot(t, x, label = "$x$")
ax[0].plot(t, y, label = "$y$")
ax[0].plot(t, z, label = "$z$")
ax[0].legend(loc = 'lower left')
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
ax[1].plot(t, tilt, 'bo')
for i in range(0, duration * 10):
    ax[1].plot([t[i], t[i]], [0,tilt[i]], c = "blue")
ax[1].plot([0, duration], [0, 0], c = "red")
ax[1].set_xlabel('Time')
ax[1].set_ylabel('Is over 5cm')
plt.show()
s.close()