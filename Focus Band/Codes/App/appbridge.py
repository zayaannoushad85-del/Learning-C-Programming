import serial
import json
import requests
import time
import math

SERIAL_PORT = "COM6"  # change
BAUD_RATE = 9600
FLASK_URL = "http://127.0.0.1:5000/update"

# smoothing
ALPHA = 0.75               # motion smoothing
JERK_ALPHA = 0.6
TALK_WINDOW = 10           # number of samples for talking detection
SUDDEN_JERK_THRESHOLD = 0.05

smoothed_motion = 0
smoothed_jerk = 0
last_motion = None

talk_history = []          # for talking probability

def classify(motion):
    global smoothed_motion, smoothed_jerk, last_motion, talk_history

    if motion is None:
        return "idle"

    # 1. Smooth motion
    if smoothed_motion == 0:
        smoothed_motion = motion
    else:
        smoothed_motion = (ALPHA * smoothed_motion) + ((1 - ALPHA) * motion)

    # 2. Compute jerk
    if last_motion is None:
        jerk = 0
    else:
        jerk = abs(motion - last_motion)

    # Smooth jerk
    smoothed_jerk = (JERK_ALPHA * smoothed_jerk) + ((1 - JERK_ALPHA) * jerk)

    last_motion = motion

    # 3. Talking detection (tiny vibration patterns)
    talk_history.append(jerk)
    if len(talk_history) > TALK_WINDOW:
        talk_history.pop(0)

    talk_score = sum(1 for j in talk_history if 0.005 < j < 0.03)

    is_talking = (talk_score > TALK_WINDOW * 0.6)

    # 4. Sudden movement detection
    sudden_move = smoothed_jerk > SUDDEN_JERK_THRESHOLD

    # 5. Classification rules (smart)
    if motion < 0.02 and not sudden_move:
        return "focused"

    if is_talking:
        return "talking"

    if sudden_move:
        return "distracted"

    if motion < 0.10:
        return "focused"

    return "distracted"

# ------------ BRIDGE MAIN LOOP --------------
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
print("Bridge running with AI-style classification...")

while True:
    try:
        raw = ser.readline().decode().strip()
        if not raw:
            continue

        print("RAW:", raw)

        # Try JSON first
        try:
            data = json.loads(raw)
        except:
            parts = raw.split(",")
            if len(parts) == 4:
                ax = float(parts[0])
                ay = float(parts[1])
                az = float(parts[2])
                motion = float(parts[3])
                data = {"ax": ax, "ay": ay, "az": az, "motion": motion}
            else:
                print("Invalid:", raw)
                continue

        motion = data.get("motion")
        status = classify(motion)

        data["status"] = status

        r = requests.post(FLASK_URL, json=data)
        print("Flask:", r.json())

    except Exception as e:
        print("ERR:", e)
