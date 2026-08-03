# ESP32 Phone IMU-Based Wi-Fi Kart

Control a two-wheel ESP32-powered Cart using your smartphone's built-in motion sensors (IMU). The smartphone acts as a wireless steering wheel, transmitting real-time orientation data over Wi-Fi to the ESP32, which calculates motor speeds for smooth differential steering.

---

## 🚀 Features

- 📱 Smartphone IMU-based steering
- 🌐 ESP32 hosted web dashboard
- 🚗 Differential drive control
- 🔊 Reverse parking buzzer
- 🚦 Start / Stop controls
- 🛑 Emergency brake
- 📊 Live motion dashboard
- 📡 Wi-Fi communication (Access Point mode)
- 📱 Mobile-friendly interface

---

## 🛠 Hardware

- ESP32 Development Board
- L298N Motor Driver
- 2 × DC Gear Motors
- Ultrasonic Distance Sensor
- Passive Buzzer
- Battery Pack
- Two-wheel Robot Chassis
- Smartphone (with accelerometer & gyroscope)

---

## 💻 Software

- Arduino IDE
- ESP32 Arduino Core
- HTML5
- CSS3
- JavaScript (Device Orientation API)

---

## ⚙️ How It Works

1. The ESP32 starts as a Wi-Fi Access Point.
2. The smartphone connects to the ESP32 network.
3. The control webpage is opened in the phone's browser.
4. JavaScript reads the phone's IMU data (Pitch & Roll).
5. Orientation data is sent to the ESP32 using HTTP/WebSocket.
6. The ESP32 converts the values into differential motor speeds.
7. The Cart moves according to the phone's tilt in real time.

---

## 📡 System Architecture

```
        Smartphone
   (Accelerometer + Gyroscope)
              │
      Wi-Fi (HTTP/WebSocket)
              │
           ESP32 Controller
              │
      L298N Motor Driver
        ┌──────────────┐
        │              │
   Left Motor      Right Motor
```

---

## 🎮 Controls

| Phone Motion | Cart Action |
|--------------|-------------|
| Tilt Forward | Move Forward |
| Tilt Backward | Reverse |
| Tilt Left | Turn Left |
| Tilt Right | Turn Right |
| Phone Level | Stop |

Additional controls available from the web dashboard:

- ▶️ Start
- ⏹ Stop
- 🛑 Emergency Brake

---

## 📈 Future Improvements

- PID-based steering control
- OTA firmware updates
- GPS navigation
- Live camera streaming
- Obstacle avoidance
- Bluetooth control mode
- Battery percentage estimation
- Speed control modes
- Data logging and telemetry

---

## 📄 License

This project is licensed under the MIT License.

---

## ⭐ Contributing

Contributions, suggestions, and pull requests are welcome.

If you find this project useful, consider giving it a ⭐ on GitHub.
