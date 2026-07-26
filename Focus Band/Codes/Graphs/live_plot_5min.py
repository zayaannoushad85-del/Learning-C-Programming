import serial
import time
import matplotlib.pyplot as plt
from collections import deque
import math

# ------------ CONFIGURE THIS ----------------
SERIAL_PORT = "COM9"  # change to your Arduino port
BAUD_RATE = 9600
POINTS_TO_SHOW = 3000  # last 5 minutes (10 readings/sec * 300 sec)
# --------------------------------------------

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # wait for Arduino to reset

# Rolling buffers
timestamps = deque(maxlen=POINTS_TO_SHOW)
accel_mag = deque(maxlen=POINTS_TO_SHOW)

plt.ion()
fig, ax = plt.subplots()

while True:
    try:
        line = ser.readline().decode("utf-8").strip()
        if not line or line.startswith("timestamp"):
            continue

        parts = line.split(",")
        if len(parts) < 9:
            continue

        ts = int(parts[0])
        ax_val = int(parts[2])
        ay_val = int(parts[3])
        az_val = int(parts[4])

        mag = math.sqrt(ax_val**2 + ay_val**2 + az_val**2)

        timestamps.append(ts)
        accel_mag.append(mag)

        ax.clear()
        ax.plot(timestamps, accel_mag)
        ax.set_title("Live Head Motion (Last 5 Minutes)")
        ax.set_xlabel("Timestamp (ms)")
        ax.set_ylabel("Accel Magnitude (raw units)")
        plt.tight_layout()
        plt.pause(0.001)

    except KeyboardInterrupt:
        print("Stopped by user")
        break
    except Exception as e:
        print("Error:", e)
        continue

ser.close()
