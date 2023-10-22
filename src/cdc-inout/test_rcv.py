import serial
import serial.tools.list_ports

import time


def find_pico_port():
    pico_vid = 0xCafe
    pico_pid = 0x4001

    # List available serial ports
    available_ports = list(serial.tools.list_ports.comports())

    for port_info in available_ports:
        if port_info.vid == pico_vid and port_info.pid == pico_pid:
            return port_info.device

    return None

# Find the Pico's COM port
pico_port = find_pico_port()

if not pico_port:
    print("Could not find pico port")
    exit(0)
else:
    print(f"Pico port = {pico_port}")


# Configure the serial port settings
ser = serial.Serial(
    port=pico_port,
    baudrate=9600,
    bytesize=8,
    parity='N',
    stopbits=1,
    timeout=1
)

try:
    # Open the serial connection
    if not ser.isOpen():
        print("Opening...")
        ser.open()

    if ser.is_open:
        print("Serial connection is open.")
        start = time.monotonic()
        i = 0
        while True:
            # Read and print the response
            ser.write(b"\x00")
            rcv_bytes = ser.read(8)
            int_value = int.from_bytes(rcv_bytes, byteorder='little', signed=False)
            print(f"[{i:5d}] rcv  {int_value} -- {' '.join(['{:02X}'.format(byte) for byte in rcv_bytes])}")
            i += 1
            if i > 10000000:
                print(8 * i / 1000 / (time.monotonic() - start), "KB/s")
                break
            


except Exception as e:
    print(f"Error: {str(e)}")

finally:
    # Close the serial connection
    if ser.is_open:
        ser.close()
        print("Serial connection is closed.")
