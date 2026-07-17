import queue
import threading
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

SERIAL_PORT = "COM6"  # Windows example
# SERIAL_PORT = "/dev/ttyUSB0"  # Linux example

BAUD_RATE = 921600  # Must match esp8266_tcp_server Serial.begin()

ser = serial.Serial(port=SERIAL_PORT, baudrate=BAUD_RATE, timeout=1)

print(f"Listening on {SERIAL_PORT} @ {BAUD_RATE} baud...")

# -----------------------------
# Telemetry Parser  (uplink)
# -----------------------------


def parse_and_push(line: str) -> None:
    """Parse a telemetry line and push it to Firebase."""
    parts = [p.strip() for p in line.split(",")]
    if not parts:
        return

    timestamp = datetime.now(timezone.utc).isoformat()

    try:
        # ── MOTOR FORMAT: MOTOR1,rpm,temp,current,airspeed ───────────────────
        if parts[0] in ["MOTOR1", "MOTOR2", "MOTOR3", "MOTOR4"] and len(parts) == 5:
            motor = parts[0]
            data = {
                "rpm": float(parts[1]),
                "temp": float(parts[2]),
                "current": float(parts[3]),
                "airspeed": float(parts[4]),
                "timestamp": timestamp,
            }
            db.reference(f"live/motors/{motor}").set(data)
            db.reference(f"history/motors/{motor}").push(data)
            print(f"[{motor}] Uploaded")

        # ── LOAD FORMAT: LOAD,val1,val2,val3 (case-insensitive) ──────────────
        elif parts[0].upper() == "LOAD" and len(parts) == 4:
            data = {
                "load1": float(parts[1]),
                "load2": float(parts[2]),
                "load3": float(parts[3]),
                "timestamp": timestamp,
            }
            db.reference("live/loadcells").set(data)
            db.reference("history/loadcells").push(data)
            print("[LOAD] Uploaded")

        else:
            print(f"Unknown packet: {line}")

    except ValueError:
        print(f"Invalid numeric data: {line}")
    except Exception as e:
        print(f"Firebase error: {e}")


# -----------------------------
# Throttle Command  (downlink)
# -----------------------------


def on_throttle_command(event) -> None:
    """
    Firebase listener: commands/throttle
    Expected schema:
        { m1: 1100, m2: 1100, m3: 1100, m4: 1100, trigger: true, ts: <epoch> }
    Writes a THROTTLE,m1,m2,m3,m4 line to Serial so the main ESP8266
    broadcasts it to all TCP clients, including the motor sub-server.
    """
    data = event.data
    if not data or not data.get("trigger"):
        return

    try:
        m1 = int(data.get("m1", 1100))
        m2 = int(data.get("m2", 1100))
        m3 = int(data.get("m3", 1100))
        m4 = int(data.get("m4", 1100))

        # Clamp to safe range
        def clamp(v):
            return max(1100, min(1940, v))

        m1, m2, m3, m4 = clamp(m1), clamp(m2), clamp(m3), clamp(m4)

        packet = f"THROTTLE,{m1},{m2},{m3},{m4}\n"
        ser.write(packet.encode("utf-8"))
        print(f"[THROTTLE] Sent → {packet.strip()}")

        # Clear trigger so repeated Firebase snaps don't re-fire the same command
        db.reference("commands/throttle/trigger").set(False)

    except Exception as e:
        print(f"Throttle command error: {e}")


# Attach the Firebase listener for throttle commands
db.reference("commands/throttle").listen(on_throttle_command)

# -----------------------------
# Async Firebase Worker
# -----------------------------
# Serial reads must never block on network I/O. The main loop hands each
# line off to this queue instantly; a background thread does the actual
# Firebase pushes.

telemetry_queue = queue.Queue()


def firebase_worker() -> None:
    while True:
        line = telemetry_queue.get()
        parse_and_push(line)
        telemetry_queue.task_done()


threading.Thread(target=firebase_worker, daemon=True).start()

# -----------------------------
# Main Loop  (uplink Serial read)
# -----------------------------

try:
    while True:
        line = ser.readline().decode("utf-8", errors="replace").strip()
        if not line:
            continue
        print("RX:", line)
        telemetry_queue.put(line)  # hand off instantly, never blocks on network

except KeyboardInterrupt:
    print("\nStopping bridge...")

finally:
    ser.close()
    print("Serial port closed.")
