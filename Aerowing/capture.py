import numpy as np
import serial

# config
PORT = str(input("PORT: "))  # Change to your port
BAUD = int(input("BAUD RATE: "))  # Change to match your device

ser = serial.Serial(PORT, BAUD, timeout=1)
print(f"Connected to {PORT} at {BAUD} baud.")

while True:
    response = ser.readline().decode("utf-8", errors="replace").strip()

    # load_values = np.empty((0, 3))

    # loads = np.array(loads).reshape(1, -1)  # force (1, N) before vstack

    # all_loads = np.vstack((load_values, loads))

    # print(loads)

    if response:
        print(f"[RX] {response}")
