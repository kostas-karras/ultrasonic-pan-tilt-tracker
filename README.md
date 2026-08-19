# ultrasonic-pan-tilt-tracker
An intelligent, 2-axis (Pan/Tilt) target tracking system built with Arduino. Unlike simple radars, this system scans the room, anticipates target movement using predictive algorithms, and uses "Range Gating" to lock onto a specific target while ignoring background noise.

## 🚀 Features
* **Predictive Tracking (EMA):** Uses an Exponential Moving Average algorithm to calculate the target's momentum and predict its next position for smoother tracking. *(Added in V2!)* 
* **Digital Double-Check Filter:** Replaces standard median filtering with a custom double-ping logic. It eliminates false echoes and introduces precise delays to prevent strong servos from jittering. *(Added in V2!)*
* **Dynamic Range Gating (±10cm):** Once a target is locked, it tracks only that specific object's depth profile, effectively ignoring background objects (like walls or desks).
* **Raster Scan Algorithm:** Scans the space systematically in a 2D zig-zag grid to acquire or re-acquire a target.
* **Auto-Centering:** Safely resets to a default position if the target is permanently lost.

## 🛠️ Hardware Requirements
* 1x Arduino Uno / Nano
* 2x Micro Servos (SG90) for Pan and Tilt axes
* 1x HC-SR04 Ultrasonic Sensor
* 1x Pan-Tilt Bracket
* 1x External Breadboard Power Supply (e.g., MB102) with a 9V wall adapter

## 🔌 Wiring Guide
* **Servo X (Pan):** Pin 6
* **Servo Y (Tilt):** Pin 3
* **HC-SR04 Trig:** Pin 12
* **HC-SR04 Echo:** Pin 10

> **⚠️ IMPORTANT POWER WARNING:** Do not power the servos directly from the Arduino 5V pin. Use the external power supply for the servos' positive (+) and negative (-) wires. You **must** connect the Arduino's `GND` pin to the negative (-) rail of the external power supply (Common Ground), otherwise the servos will behave erratically.

*(Built with the built-in `Servo.h` and the `NewPing.h` libraries)*
