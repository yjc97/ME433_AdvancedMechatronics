# -*- coding: utf-8 -*-
"""
Created on Wed Apr 27 17:47:43 2022

@author: jeff8
"""

# read data from the imu and plot

# sudo apt-get install python3-pip
# python -m pip install pyserial
# sudo apt-get install python-matplotlib

import serial
ser = serial.Serial('COM4',230400)
print('Opening port: ')
print(ser.name)

read_samples = 10 # anything bigger than 1 to start out
ax = []
ay = []
az = []
gx = []
gy = []
gz = []
temp = []
print('Requesting data collection...')
ser.write(b'\n')
while read_samples > 1:
    data_read = ser.read_until(b'\n',200) # get the data as bytes
    data_text = str(data_read,'utf-8') # turn the bytes to a string
    data = [float(i) for i in data_text.split()] # turn the string into a list of floats

    if(len(data)==8):
        read_samples = int(data[0]) # keep reading until this becomes 1
        ax.append(data[1])
        ay.append(data[2])
        az.append(data[3])
        gx.append(data[4])
        gy.append(data[5])
        gz.append(data[6])
        temp.append(data[7])
print('Data collection complete')

#complementary_filter
import math

dt = 0.01
A = 0.1
accPitch = []
accRoll = []
gyroPitch = []
gyroRoll = []
roll = []
pitch = []

for i in range(len(gx)):
    
    # Acceleration vector angle
    accPitch.append(math.degrees(math.atan2(ay[i],az[i])))
    accRoll.append(math.degrees(math.atan2(ax[i],az[i])))
    if i == 0:
        gyroRoll.append(gy[0]*dt)
        gyroPitch.append(gx[0]*dt)
    if i > 0:
        # Gyro integration angle
        gyroRoll.append( gyroRoll[i-1] + gy[i-1] * dt)
        gyroPitch.append( gyroPitch[i-1] +gx[i-1] * dt)

# Comp filter
for index in range(len(gyroRoll)):
    
    roll.append(A*(gyroRoll[index]) + (1-A)*(accRoll[index]))
    pitch.append( A*(gyroPitch[index]) + (1-A)*(accPitch[index]))
        
    


# plot it
import matplotlib.pyplot as plt 
t = range(len(ax)) # time array
plt.plot(t,ax,'r*-',t,ay,'b*-',t,az,'k*-')
plt.ylabel('G value')
plt.xlabel('sample')
plt.show()

t = range(len(gx)) # time array
plt.plot(t,gx,'r*-',t,gy,'b*-',t,gz,'k*-')
plt.ylabel('Omega value')
plt.xlabel('sample')
plt.show()

t = range(len(temp)) # time array
plt.plot(t,temp,'r*-')
plt.ylabel('Temperature value')
plt.xlabel('sample')
plt.show()

t = range(len(gx))
plt.plot(t,roll,'r*-',t,pitch,'b*-')
plt.ylabel('Omega value w/ filter')
plt.xlabel('sample')
plt.show()

# be sure to close the port
ser.close()