import serial
import time
import matplotlib.pyplot as plt
from collections import deque

# ---------------- CONFIGURE THIS ----------------
SERIAL_PORT = "COM9"   # change if needed
BAUD_RATE = 9600
POINTS_TO_SHOW = 3000
# ------------------------------------------------

# Connect to Arduino
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)

# Buffers for storing data
timestamps = deque(maxlen=POINTS_TO_SHOW)
pulse_data = deque(maxlen=POINTS_TO_SHOW)
motion_data = deque(maxlen=POINTS_TO_SHOW)

# Set up live plotting
plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6), sharex=True)

line1, = ax1.plot([], [], label="Pulse")
line2, = ax2.plot([], [], label="Motion")

ax1.set_title("Pulse Sensor Readings")
ax2.set_title("Motion (MPU6050) Readings")
ax1.set_ylabel("Pulse")
ax2.set_ylabel("Motion")
ax2.set_xlabel("Time (seconds)")

start_time = time.time()

print("🔄 Reading and plotting Pulse & Motion data...")

while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        # Expecting: "pulse,motion" from Arduino
        parts = line.split(',')
        if len(parts) != 2:
            print(f"Skipping invalid line: {line}")
            continue

        pulse = float(parts[0])
        motion = float(parts[1])

        current_time = time.time() - start_time
        timestamps.append(current_time)
        pulse_data.append(pulse)
        motion_data.append(motion)

        line1.set_xdata(timestamps)
        line1.set_ydata(pulse_data)

        line2.set_xdata(timestamps)
        line2.set_ydata(motion_data)

        ax1.relim()
        ax1.autoscale_view()
        ax2.relim()
        ax2.autoscale_view()

        plt.pause(0.01)

    except ValueError:
        print(f"Skipping invalid line: {line}")
    except Exception as e:
        print("Error:", e)
        break
