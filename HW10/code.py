from ulab import numpy as np
import time

x = np.linspace(0, 100, num=1024)
y  = np.sin(x)
y2 = np.sin(2*x)
y3 = np.sin(3*x)
y4 = []
z = np.zeros(len(x))


for i,j,k in zip(y,y2,y3):
    y4.append(i+j+k)
y4_np = np.array(y4)
a,b = np.fft.fft(y4_np) #a is real part, b is imagenary part

while 1:
    for i,j,k in zip(y4,a,b):
        print((i,j,k)) # print with plotting format
        time.sleep(.05) # delay in seconds
