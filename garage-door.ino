#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ================== CONFIGURATION ==================
#define RELAY_PIN 23            // Connected to IN1 on the relay module
#define RELAY_ACTIVE_LOW 1      // 1 if the relay is triggered by a LOW signal
const unsigned long PULSE_MS = 700;   // Pulse duration in milliseconds

const char* ssid     = "WiFi_SSID";
const char* password = "WiFi_Password";

AsyncWebServer server(80);

// Non-blocking pulse state
bool pulseActive = false;
unsigned long pulseEnd = 0;

// ================== RELAY CONTROL ==================
inline void setIdle() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);  // Relay OFF
}

inline void setOn() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);  // Relay ON
}

void startPulse() {
  setOn();
  pulseActive = true;
  pulseEnd = millis() + PULSE_MS;
}

void servicePulse() {
  if (pulseActive && millis() >= pulseEnd) {
    setIdle();
    pulseActive = false;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Default state: relay off
  pinMode(RELAY_PIN, INPUT_PULLUP);
  setIdle();

  // --- Wi-Fi connection ---
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  Serial.print("[WiFi] Connecting to "); Serial.println(ssid);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 60000) {
    delay(250); Serial.print('.');
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Connection failed, restarting...");
    ESP.restart();
  }
  Serial.print("\n[WiFi] Connected. IP address: "); Serial.println(WiFi.localIP());

  // --- Web Server ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
    r->send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <meta charset="utf-8"/>
        <title>Garage Control</title>
        <script>
          async function sendPulse() {
            const r = await fetch('/pulse');
            alert(await r.text());
          }
        </script>
      </head>
      <body style="font-family:sans-serif;text-align:center;margin-top:2em">
        <h1>Garage Door Control</h1>
        <button onclick="sendPulse()" style="font-size:2em;padding:1em 2em;">Open / Close</button>
      </body>
      </html>
    )rawliteral");
  });

  server.on("/pulse", HTTP_GET, [](AsyncWebServerRequest *r) {
    startPulse();
    r->send(200, "text/plain", "Pulse sent");
  });

  server.begin();
  Serial.println("[HTTP] Server started");
}

// ================== LOOP ==================
void loop() {
  servicePulse();   // Check if the pulse duration has ended
  delay(1);
}
