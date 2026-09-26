#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

// ================== CONFIGURATION ==================
#define RELAY_PIN 23
#define RELAY_ACTIVE_LOW 1

const unsigned long PULSE_MS = 700;
const unsigned long WIFI_TIMEOUT_MS = 30000;

// Local hostname: http://garage-thulin.local
const char* HOSTNAME = "garage-thulin";

// Optional fallback Wi-Fi credentials compiled into the firmware.
// Leave DEFAULT_SSID empty to use only credentials saved from the setup page.
const char* DEFAULT_SSID = "WiFi_SSID";
const char* DEFAULT_PASSWORD = "WiFi_Password";

// Protected Wi-Fi setup access point.
// WPA2 requires a password of at least 8 characters.
const char* SETUP_AP_SSID = "Garage-Thulin-Setup";
const char* SETUP_AP_PASSWORD = "CHANGE-ME-SETUP-PASSWORD";

// Credentials protecting the normal garage web interface and /pulse endpoint.
const char* WEB_USERNAME = "garage";
const char* WEB_PASSWORD = "CHANGE-ME-WEB-PASSWORD";

// Password used only for wireless firmware updates (Arduino OTA).
// Keep it different from the web and setup passwords.
const char* OTA_PASSWORD = "CHANGE-ME-OTA-PASSWORD";

AsyncWebServer server(80);
Preferences preferences;

bool pulseActive = false;
unsigned long pulseEnd = 0;

// ================== RELAY CONTROL ==================
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
  if (pulseActive && (long)(millis() - pulseEnd) >= 0) {
    setIdle();
    pulseActive = false;
  }
}

// ================== WIFI ==================
bool connectToWiFi(const char* ssid, const char* password, unsigned long timeoutMs) {
  if (ssid == nullptr || strlen(ssid) == 0) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.persistent(false);
  WiFi.begin(ssid, password);

  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < timeoutMs) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connection failed.");
    WiFi.disconnect(true);
    delay(250);
    return false;
  }

  WiFi.setAutoReconnect(true);
  Serial.print("[WiFi] Connected. IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void startMDNS() {
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("[mDNS] Failed to start.");
    return;
  }

  MDNS.addService("http", "tcp", 80);
  Serial.print("[mDNS] Available at http://");
  Serial.print(HOSTNAME);
  Serial.println(".local");
}

// ================== OTA UPDATES ==================
void startOTA() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    // Never leave the relay energized while replacing the firmware.
    setIdle();
    pulseActive = false;
    Serial.println("[OTA] Update started. Relay forced OFF.");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Update complete. Rebooting...");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percent = total ? (progress * 100U) / total : 0;
    Serial.printf("[OTA] Progress: %u%%\r", percent);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("\n[OTA] Error %u: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("authentication failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("connection failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("receive failed");
    else if (error == OTA_END_ERROR) Serial.println("end failed");
    else Serial.println("unknown error");
  });

  ArduinoOTA.begin();

  Serial.print("[OTA] Ready as ");
  Serial.print(HOSTNAME);
  Serial.println(".local");
}

// ================== NORMAL WEB SERVER ==================
void startNormalServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(WEB_USERNAME, WEB_PASSWORD)) {
      return request->requestAuthentication();
    }

    request->send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Garage Thulin</title>
  <style>
    body { font-family: sans-serif; text-align: center; margin: 0; padding: 40px 20px; background: #f5f5f5; }
    .card { max-width: 400px; margin: auto; padding: 30px; background: white; border-radius: 16px; box-shadow: 0 4px 20px rgba(0,0,0,.1); }
    button { width: 100%; padding: 20px; font-size: 1.5rem; border: 0; border-radius: 12px; cursor: pointer; }
    #status { min-height: 1.5em; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="card">
    <h1>Garage</h1>
    <button id="garageButton" onclick="sendPulse()">Ouvrir / Fermer</button>
    <div id="status"></div>
  </div>
  <script>
    async function sendPulse() {
      const button = document.getElementById('garageButton');
      const status = document.getElementById('status');
      button.disabled = true;
      status.textContent = 'Commande en cours...';

      try {
        const response = await fetch('/pulse', { method: 'POST' });
        if (!response.ok) throw new Error('HTTP ' + response.status);
        status.textContent = 'Commande envoyée.';
      } catch (error) {
        status.textContent = 'Erreur : ' + error.message;
      } finally {
        setTimeout(() => {
          button.disabled = false;
          status.textContent = '';
        }, 1500);
      }
    }
  </script>
</body>
</html>
)rawliteral");
  });

  server.on("/pulse", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(WEB_USERNAME, WEB_PASSWORD)) {
      return request->requestAuthentication();
    }

    if (pulseActive) {
      request->send(409, "application/json", "{\"success\":false,\"error\":\"pulse_already_active\"}");
      return;
    }

    startPulse();
    request->send(200, "application/json", "{\"success\":true}");
  });

  server.begin();
  Serial.println("[HTTP] Protected garage server started.");
}

// ================== SETUP MODE ==================
void startSetupMode() {
  Serial.println();
  Serial.println("============================");
  Serial.println("      WIFI SETUP MODE");
  Serial.println("============================");

  WiFi.disconnect(true);
  delay(250);
  WiFi.mode(WIFI_AP);

  if (!WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASSWORD)) {
    Serial.println("[Setup] Failed to start access point.");
    return;
  }

  Serial.print("[Setup] SSID: ");
  Serial.println(SETUP_AP_SSID);
  Serial.print("[Setup] Open http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Garage - Configuration Wi-Fi</title>
  <style>
    body { font-family: sans-serif; background: #f5f5f5; padding: 30px 20px; }
    .card { max-width: 420px; margin: auto; background: white; padding: 30px; border-radius: 16px; box-shadow: 0 4px 20px rgba(0,0,0,.1); }
    label { display: block; margin-top: 20px; margin-bottom: 5px; }
    input { width: 100%; box-sizing: border-box; padding: 14px; font-size: 1rem; border: 1px solid #ccc; border-radius: 8px; }
    button { width: 100%; margin-top: 25px; padding: 15px; font-size: 1.1rem; border: 0; border-radius: 8px; cursor: pointer; }
    .info { color: #666; font-size: .9rem; }
  </style>
</head>
<body>
  <div class="card">
    <h1>Configuration Wi-Fi</h1>
    <p>Le contrôleur n'arrive pas à se connecter au réseau Wi-Fi.</p>
    <form action="/save" method="POST">
      <label for="ssid">Nom du réseau Wi-Fi</label>
      <input id="ssid" name="ssid" type="text" maxlength="32" required>
      <label for="password">Mot de passe Wi-Fi</label>
      <input id="password" name="password" type="password" maxlength="64">
      <button type="submit">Enregistrer et redémarrer</button>
    </form>
    <p class="info">Les identifiants seront enregistrés dans la mémoire de l'ESP32.</p>
  </div>
</body>
</html>
)rawliteral");
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("ssid", true)) {
      request->send(400, "text/plain", "SSID missing");
      return;
    }

    String ssid = request->getParam("ssid", true)->value();
    String password = request->hasParam("password", true)
      ? request->getParam("password", true)->value()
      : "";

    ssid.trim();
    if (ssid.length() == 0 || ssid.length() > 32 || password.length() > 64) {
      request->send(400, "text/plain", "Invalid Wi-Fi credentials");
      return;
    }

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();

    request->send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Configuration enregistrée</title>
</head>
<body style="font-family:sans-serif;text-align:center;padding:40px">
  <h1>Configuration enregistrée</h1>
  <p>Le contrôleur va redémarrer.</p>
  <p>Reconnectez-vous ensuite à votre Wi-Fi et ouvrez <strong>http://garage-thulin.local</strong>.</p>
</body>
</html>
)rawliteral");

    delay(1500);
    ESP.restart();
  });

  server.begin();
  Serial.println("[Setup] Protected setup access point started.");
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Keep the relay explicitly off during boot and Wi-Fi setup.
  pinMode(RELAY_PIN, INPUT_PULLUP);
  setIdle();

  preferences.begin("wifi", true);
  String storedSSID = preferences.getString("ssid", "");
  String storedPassword = preferences.getString("password", "");
  preferences.end();

  bool connected = false;

  if (storedSSID.length() > 0) {
    Serial.println("[WiFi] Trying stored credentials...");
    connected = connectToWiFi(storedSSID.c_str(), storedPassword.c_str(), WIFI_TIMEOUT_MS);
  }

  if (!connected && strlen(DEFAULT_SSID) > 0 && strcmp(DEFAULT_SSID, "WiFi_SSID") != 0) {
    Serial.println("[WiFi] Trying default credentials...");
    connected = connectToWiFi(DEFAULT_SSID, DEFAULT_PASSWORD, WIFI_TIMEOUT_MS);
  }

  if (connected) {
    startMDNS();
    startOTA();
    startNormalServer();
  } else {
    startSetupMode();
  }
}

// ================== LOOP ==================
void loop() {
  // OTA is only started in normal station mode, never on the setup AP.
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }

  servicePulse();
  delay(1);
}
