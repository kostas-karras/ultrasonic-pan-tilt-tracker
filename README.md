# ultrasonic-pan-tilt-tracker
An intelligent, 2-axis (Pan/Tilt) target tracking system built with Arduino. Unlike simple radars, this system scans the room and uses a "Range Gating" algorithm to lock onto a specific target, ignoring background noise.

## 🚀 Features
* **Raster Scan Algorithm:** Scans the space systematically in a 2D grid to acquire a target.
* **Dynamic Range Gating (±10cm):** Once a target is locked, it tracks only that specific object's depth profile, effectively ignoring background objects (like walls or desks).
* **Digital Noise Filtering:** Uses the `ping_median()` method to ignore false echoes and sensor glitches for extremely reliable tracking.
* **Auto-Centering:** Safely resets to a default position if the target is permanently lost.

## 🛠️ Hardware Requirements
* 1x Arduino Uno / Nano
* 2x Micro Servos (SG90)
* 1x HC-SR04 Ultrasonic Sensor

## 🔌 Wiring Guide
 Servo X (Pan) -> Pin 6  
 Servo Y (Tilt) -> Pin 3   
 HC-SR04 Trig -> Pin 12   
 HC-SR04 Echo -> Pin 10   

*(Built with the built-in `Servo.h` and the `NewPing.h` libraries)*
