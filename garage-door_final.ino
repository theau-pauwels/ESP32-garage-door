#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ================== CONFIGURATION ==================
#define RELAY_PIN 23           // IN1 du module relais
#define RELAY_ACTIVE_LOW 1     // 1 si le relais s'active avec un niveau bas
const unsigned long PULSE_MS = 700;   // Durée de l'impulsion en ms

const char* ssid     = "WiFi_SSID";
const char* password = "WiFi_Password";

AsyncWebServer server(80);

// État d'impulsion non bloquante
bool pulseActive = false;
unsigned long pulseEnd = 0;

// ================== RELAIS ==================
inline void setIdle() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

inline void setOn() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
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

  // Repos par défaut
  pinMode(RELAY_PIN, INPUT_PULLUP);
  setIdle();

  // --- Connexion Wi-Fi ---
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  Serial.print("[WiFi] Connexion à "); Serial.println(ssid);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 60000) {
    delay(250); Serial.print('.');
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Échec, redémarrage...");
    ESP.restart();
  }
  Serial.print("\n[WiFi] IP: "); Serial.println(WiFi.localIP());

  // --- Serveur Web ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
    r->send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head><meta charset="utf-8"/><title>Garage</title>
      <script>
        async function sendPulse(){
          const r = await fetch('/pulse');
          alert(await r.text());
        }
      </script></head>
      <body style="font-family:sans-serif;text-align:center;margin-top:2em">
        <h1>Contrôle de la porte de garage</h1>
        <button onclick="sendPulse()" style="font-size:2em;padding:1em 2em;">Ouvrir / Fermer</button>
      </body>
      </html>
    )rawliteral");
  });

  server.on("/pulse", HTTP_GET, [](AsyncWebServerRequest *r) {
    startPulse();
    r->send(200, "text/plain", "Impulsion envoyée");
  });

  server.begin();
  Serial.println("[HTTP] Serveur démarré");
}

// ================== LOOP ==================
void loop() {
  servicePulse();
  delay(1);
}
