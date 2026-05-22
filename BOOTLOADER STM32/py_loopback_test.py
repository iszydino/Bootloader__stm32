import serial
import time

ser = serial.Serial(
    port="COM12",
    baudrate=9600,
    timeout=2,
    xonxoff=False,
    rtscts=False,
    dsrdtr=False
)

time.sleep(1)

ser.reset_input_buffer()
ser.reset_output_buffer()

ser.write(b'X')
ser.flush()

time.sleep(0.2)

resp = ser.read(8)
print(repr(resp))

ser.close()