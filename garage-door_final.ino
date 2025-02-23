#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Définir les broches utilisées
#define PIN_D22 22
#define PIN_D23 23

// Informations Wi-Fi
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// Créer un serveur web
AsyncWebServer server(80);

// Fonction pour générer une impulsion différentielle
void sendDifferentialPulse() {
  digitalWrite(PIN_D22, HIGH);
  digitalWrite(PIN_D23, HIGH);
  delay(500);  // Durée de l'impulsion (ajustez selon vos besoins)
  digitalWrite(PIN_D22, LOW);
  digitalWrite(PIN_D23, LOW);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Attendre que la connecion série soit up

  // Initialiser les broches
  pinMode(PIN_D22, OUTPUT);
  pinMode(PIN_D23, OUTPUT);

  // Initialiser les broches à LOW
  digitalWrite(PIN_D22, LOW);
  digitalWrite(PIN_D23, LOW);

///// WiFi Setup
  // Connexion au réseau WiFi
  WiFi.begin(ssid, password);
  int trial = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    trial = trial + 1;
    if (trial == 120){
      ESP.restart();
    }
  }
  Serial.println("\n");
  Serial.println("Connecté à l'adresse: ");
  Serial.println(WiFi.localIP());
///// End WiFi

  // Démarrer le serveur
  server.begin();

  // Configurer le serveur pour gérer une requête d'impulsion
  //server.on("/pulse", HTTP_GET, [](AsyncWebServerRequest *request) {
  //  sendDifferentialPulse();
  //  request->send(200, "text/plain", "Impulse sent!");
  //});
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>Contrôle ESP32</title>
        <script>
          function sendPulse() {
            fetch('/pulse')
              .then(response => {
                if (!response.ok) throw new Error("Erreur lors de l'envoi !");
                return response.text();
              })
              .then(alert)
              .catch(err => alert("Erreur : " + err.message));
          }
        </script>
      </head>
      <body>
        <h1>Contrôle des impulsions</h1>
        <button onclick="sendPulse()">Envoyer une impulsion</button>
      </body>
      </html>
    )rawliteral");
  });
    server.on("/pulse", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(22, HIGH);
    digitalWrite(23, HIGH);
    delay(500);  // Durée de l'impulsion
    digitalWrite(22, LOW);
    digitalWrite(23, LOW);

    request->send(200, "text/plain", "Impulsion envoyée !");
  });
}

void loop() {
  // Rien à faire dans la boucle principale
}
