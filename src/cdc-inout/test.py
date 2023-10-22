import serial
import serial.tools.list_ports


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

        # Send data to the Pico
        data_to_send = "Hello, Pico.. 1!\n"
        # ser.write(data_to_send.encode())

        interval_ms = 40
        while True:
            print(f"============ {interval_ms} ============")
            interval_bytes = interval_ms.to_bytes(4, byteorder='little', signed=False)
            ser.write(interval_bytes)
            print(f"sent {' '.join(['{:02X}'.format(byte) for byte in interval_bytes])}")

            # Read and print the response
            rcv_bytes = ser.readline()
            print(f"rcv  {' '.join(['{:02X}'.format(byte) for byte in rcv_bytes])}")
            
            if (interval_ms > 1000):
                interval_ms = 40
            # response = rcv_bytes.decode()
            # response = ser.readline().decode()
            # print(f"Received: {response}")
            
            interval_ms += 10

except Exception as e:
    print(f"Error: {str(e)}")

finally:
    # Close the serial connection
    if ser.is_open:
        ser.close()
        print("Serial connection is closed.")
