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

After the **first USB flash**, normal firmware updates can be installed over Wi-Fi without physically accessing the ESP32.

### Requirements

- The ESP32 must already be connected to the normal home Wi-Fi.
- The computer running Arduino IDE must be on the same local network.
- `OTA_PASSWORD` in the firmware must match the password used for the upload.
- OTA is deliberately **not available** while the ESP32 is running the `Garage-Thulin-Setup` access point.

### Update from Arduino IDE

1. Make the required changes to `garage-door.ino`.
2. Click **Verify** first and make sure compilation succeeds.
3. Keep the computer connected to the same LAN as the ESP32.
4. In Arduino IDE, open **Tools → Port**.
5. Wait for the network port named `garage-thulin` / `garage-thulin.local` to appear.
6. Select this **network port** instead of the USB serial port.
7. Click **Upload**.
8. Enter the value configured in `OTA_PASSWORD` when Arduino IDE requests the OTA password.
9. Wait until the upload reaches 100%. Do not power off the ESP32 or router during the update.
10. The ESP32 automatically reboots into the new firmware.

The web interface will be temporarily unavailable during the update and reboot.

### Safety during an OTA update

When an OTA update starts, the firmware:

- immediately forces the relay to its idle/OFF state;
- cancels any active relay pulse;
- writes the new firmware;
- reboots after a successful update.

This prevents the relay from intentionally remaining energized while the firmware is being replaced. As with any embedded controller connected to a garage door, keep the physical installation fail-safe and do not rely on OTA software as the only safety mechanism.

### Verify the update

The Serial Monitor is normally unavailable when the ESP32 is not connected by USB. After the update, verify that:

- `http://garage-thulin.local` responds again;
- the web authentication still works;
- the garage command works as expected.

If the ESP32 is temporarily connected by USB, the serial output at **115200 baud** also reports OTA start, progress, errors and the subsequent boot.

### If the network port does not appear

First check that `http://garage-thulin.local` is reachable from the computer. If it is, but Arduino IDE does not show the OTA port:

1. wait a few seconds and reopen **Tools → Port**;
2. confirm the computer and ESP32 are on the same LAN/VLAN;
3. check that the router/access point is not using client/AP isolation;
4. check that multicast/mDNS traffic is allowed on the local network;
5. restart Arduino IDE if discovery still does not refresh.

mDNS is used for discovery, so a network that blocks multicast can prevent the Arduino IDE from automatically finding the ESP32 even when ordinary IP connectivity works.

### Recovery

**USB remains the recovery method.** Use a USB flash again if, for example:

- the new firmware no longer connects to Wi-Fi;
- OTA initialization is broken;
- the Wi-Fi configuration has been lost and cannot be restored through the setup AP;
- an incompatible firmware was installed.

For that reason, always run **Verify** before starting an OTA upload.

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
