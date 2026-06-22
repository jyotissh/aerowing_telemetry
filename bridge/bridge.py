from datetime import datetime, timezone

import firebase_admin
import serial
from firebase_admin import credentials, db

# -----------------------------
# Firebase Initialization
# -----------------------------

cred = credentials.Certificate(
    "aerowing-telemetry-dashboard-firebase-adminsdk-fbsvc-d00038bfe7.json"
)

firebase_admin.initialize_app(
    cred,
    {
        "databaseURL": "https://aerowing-telemetry-dashboard-default-rtdb.asia-southeast1.firebasedatabase.app"
    },
)

# -----------------------------
# Serial Port Configuration
# -----------------------------

SERIAL_PORT = "COM14"  # Windows example
# SERIAL_PORT = "/dev/ttyUSB0"   # Linux example

BAUD_RATE = 921600

ser = serial.Serial(port=SERIAL_PORT, baudrate=BAUD_RATE, timeout=1)

print(f"Listening on {SERIAL_PORT} @ {BAUD_RATE} baud...")

# -----------------------------
# Telemetry Parser
# -----------------------------


def parse_and_push(line):
    parts = [p.strip() for p in line.split(",")]

    if not parts:
        return

    timestamp = datetime.now(timezone.utc).isoformat()

    try:
        # --------------------------------
        # MOTOR FORMAT:
        # MOTOR1,rpm,temp,current,airspeed
        # --------------------------------

        if parts[0] in ["MOTOR1", "MOTOR2", "MOTOR3", "MOTOR4"] and len(parts) == 5:
            motor = parts[0]

            data = {
                "rpm": float(parts[1]),
                "temp": float(parts[2]),
                "current": float(parts[3]),
                "airspeed": float(parts[4]),
                "timestamp": timestamp,
            }

            # Latest telemetry
            db.reference(f"live/motors/{motor}").set(data)

            # Historical telemetry
            db.reference(f"history/motors/{motor}").push(data)

            print(f"[{motor}] Uploaded")

        # --------------------------------
        # LOAD FORMAT:
        # LOAD,val1,val2,val3
        # --------------------------------

        elif parts[0] == "Load" and len(parts) == 4:
            data = {
                "load1": float(parts[1]),
                "load2": float(parts[2]),
                "load3": float(parts[3]),
                "timestamp": timestamp,
            }

            # Latest telemetry
            db.reference("live/loadcells").set(data)

            # Historical telemetry
            db.reference("history/loadcells").push(data)

            print("[LOAD] Uploaded")

        else:
            print(f"Unknown packet: {line}")

    except ValueError:
        print(f"Invalid numeric data: {line}")

    except Exception as e:
        print(f"Firebase error: {e}")


# -----------------------------
# Main Loop
# -----------------------------

try:
    while True:
        line = ser.readline().decode("utf-8", errors="replace").strip()

        if not line:
            continue

        print("RX:", line)

        parse_and_push(line)

except KeyboardInterrupt:
    print("\nStopping bridge...")

finally:
    ser.close()
    print("Serial port closed.")
