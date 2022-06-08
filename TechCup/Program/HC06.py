# Use the UART Friend to talk to the HC-06
import time
import serial

# the port is your UART Friend port
# the baud for the HC-06 is initially 9600
ser = serial.Serial('COM3',9600)

print('Opening port: ' + str(ser.name))

ser.write(b'AT+VERSION\r\n') # see if you get a response
#ser.write(b'AT+NAMEray1997\r\n') # change the bluetooth name to nickhc06
#ser.write(b'AT+BAUD8\r\n') # change the baud to 115200

time.sleep(1) # wait a little for the HC-06 to respond
print("Response len = " + str(ser.in_waiting))

# read all the bytes that were received 
d = ""
for i in range(ser.in_waiting):
	d = d + (str(ser.read(),'utf-8'))
	
print("Response: " + str(d))

print("Done!")

ser.close()