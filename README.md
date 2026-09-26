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
- Local hostname: `http://garage-thulin.local`
- Password-protected web interface and `POST /pulse`
- Password-protected `Garage-Thulin-Setup` access point if the saved Wi-Fi is unavailable
- Wi-Fi credentials stored in ESP32 non-volatile memory
- Authenticated Arduino OTA firmware updates over the local network
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

Before the first flash, change the placeholder secrets in `garage-door.ino`:

```cpp
const char* SETUP_AP_PASSWORD = "CHANGE-ME-SETUP-PASSWORD";
const char* WEB_USERNAME = "garage";
const char* WEB_PASSWORD = "CHANGE-ME-WEB-PASSWORD";
const char* OTA_PASSWORD = "CHANGE-ME-OTA-PASSWORD";
```

Use three different strong passwords. **Do not commit real passwords to this public repository.**

The compiled-in Wi-Fi credentials are optional. Leaving `DEFAULT_SSID` at its placeholder value makes the ESP32 start the protected setup AP when no stored Wi-Fi works.

You can adjust the relay pulse duration if needed:

```cpp
const unsigned long PULSE_MS = 700;
```

---

## 🧩 First setup

1. Flash the ESP32 once over USB with Arduino IDE.
2. If no configured Wi-Fi is available, wait about 30 seconds for `Garage-Thulin-Setup`.
3. Join that network using `SETUP_AP_PASSWORD`.
4. Browse to `http://192.168.4.1`, enter the home Wi-Fi credentials, and save.
5. Reconnect your device to the home network.
6. Open `http://garage-thulin.local` and authenticate with `WEB_USERNAME` / `WEB_PASSWORD`.

---

## 📡 Wireless firmware updates (Arduino OTA)

OTA is available only while the ESP32 is connected to the normal home Wi-Fi. It is not started on the `Garage-Thulin-Setup` access point.

After the first USB flash:

1. Connect the Ubuntu notebook to the same LAN as the ESP32.
2. Open Arduino IDE and wait for the network port for `garage-thulin` to appear under **Tools → Port**.
3. Select that network port.
4. Click **Upload**.
5. Enter `OTA_PASSWORD` when Arduino IDE asks for the OTA password.
6. The firmware is transferred over Wi-Fi and the ESP32 reboots automatically.

During an OTA update, the code forces the relay OFF before writing the firmware.

If Arduino IDE does not discover the network port, verify that the notebook and ESP32 are on the same LAN and that multicast/mDNS traffic is not isolated by the router. The ESP32 remains reachable at `garage-thulin.local` when mDNS is working.

A USB connection remains the recovery method if an OTA update installs firmware that can no longer connect to Wi-Fi or start OTA.

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
