# ESP32 Differential Pulse Web Controller

## Overview
This project sets up an ESP32-based web server that allows users to remotely send differential pulses via a web interface. The ESP32 connects to a Wi-Fi network and provides an HTML interface where users can trigger pulses on GPIO pins 22 and 23.

## Features
- Wi-Fi connectivity for remote control
- Web-based interface to send pulses
- Simple GET request endpoint (`/pulse`) to trigger pulses programmatically
- Auto-reconnect mechanism in case of connection failure

## Hardware Requirements
- ESP32 development board
- Wi-Fi network
- A relay to connect to your garage door motor
- (Optional) LED or oscilloscope to monitor pulse output

## Installation
1. Clone this repository:
   ```sh
   git clone https://github.com/yourusername/esp32-pulse-control.git

2. Open the project in the Arduino IDE.
3. Install the following dependencies:
- **WiFi.h** (included with ESP32 core for Arduino)
- **ESPAsyncWebServer.h** (install via Arduino Library Manager)
4. Update your Wi-Fi credentials in the code:
  ```sh
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
5. Upload the code to your ESP32.

## Usage
1. Once powered on, the ESP32 connects to Wi-Fi and prints its IP address to the Serial Monitor. (for debugging purpose). You better set up a static IP through your router.
2. Open a web browser and enter the displayed IP address.
3. Use the web interface to send pulses to GPIO pins 22 and 23.
4. Go to the IP of your ESP32 to send the impulse. (Alternatively, make a GET request to **/pulse** to trigger an impulse:
   ```sh
   curl http://<ESP32_IP>/pulse
)

## Code Breakdown
- Wi-Fi Setup: The ESP32 connects to the specified Wi-Fi network and retries on failure.
- Web Server:
   - **/** serves an HTML page with a button to trigger pulses.
   - **/pulse** sends a 500ms high pulse to GPIO 22 and 23.
- Differential Pulse: A function toggles the GPIO pins to generate an impulse.

## Future Improvements
- Add configurable pulse duration
- Implement authentication for secure access
- Expand control options for multiple GPIOs

## License
This project is licensed under the MIT License. Feel free to modify and use it!
