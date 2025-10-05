# ESP32 Garage Door Controller

This project allows you to control a **garage door** using an **ESP32** and a **5V relay module** through a simple web interface.  
When connected to Wi-Fi, you can open or close the door by visiting a local web page or triggering an HTTP request.

---

## 🔧 Hardware Overview

### Required components
- 1 × ESP32 DevKit (WROOM-32)
- 1 × 1 or 2-channel relay module (5V type, JD-VCC/VCC jumper ON)
- Basic jumper wires
- Your garage door’s **manual wall-switch input** (dry contact)

### Wiring Diagram (simplified)

```
      +--------------------+
      |      ESP32         |
      |                    |
      |   VIN (5V)  ------> JD-VCC / VCC (relay)
      |   GND       ------> GND (relay)
      |   GPIO 23   ------> IN1 (relay)
      |                    |
      +--------------------+

          RELAY MODULE (1CH or 2CH)
             COM ----> one wire to door control
             NO  ----> other wire to door control
```

> The relay acts as a **momentary push button** between COM and NO.

---

## ⚙️ Software Logic

### Behavior
- The ESP32 hosts a small web server on port **80**.
- Visiting the ESP32’s IP (e.g., `http://10.10.10.94/`) shows a **simple HTML page** with one button.
- Clicking the button triggers a **700 ms non-blocking pulse** on GPIO 23, simulating a short button press.

### Features
- Non-blocking timing (Wi-Fi stays responsive)
- Auto-reconnects to Wi-Fi on disconnect
- Clean HTML UI (mobile-friendly)

---

## 🧠 Code Overview

| Section | Purpose |
|----------|----------|
| **Wi-Fi setup** | Connects to your local network with auto-reconnect. |
| **Relay control** | Handles HIGH/LOW logic for active-LOW relay modules. |
| **Non-blocking pulse** | Uses `millis()` to time the 700 ms activation. |
| **Web server** | Serves a minimal HTML interface and listens for `/pulse` requests. |

---

## 🪛 Configuration

In the code, update your Wi-Fi credentials:

```cpp
const char* ssid = "Your_SSID";
const char* password = "Your_Password";
```

You can adjust pulse duration if needed:

```cpp
const unsigned long PULSE_MS = 700; // milliseconds
```

---

## 🧩 Usage

1. Upload the code to your ESP32 using the Arduino IDE.
2. Open the serial monitor to find the **assigned IP address**.
3. Visit that address in your browser (e.g., `http://10.10.10.94/`).
4. Click **Open / Close** to toggle your garage door.

---

## ⚠️ Notes

- The relay is powered from **VIN (5V)** — make sure your ESP32 is powered via USB or a 5V supply.
- JD-VCC/VCC jumper must remain **installed**.
- The relay contact (COM + NO) should connect only to the **door trigger input** (dry contact).

---

## 🧾 License

This project is released under the MIT License.  
Feel free to modify and use it for personal or educational purposes.
