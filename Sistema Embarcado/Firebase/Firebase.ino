#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// --------------------- CONFIGURAÇÕES WI-FI ---------------------
#define WIFI_SSID "Rep.Alcooltece_2.4G"    // Rede 2.4 GHz
#define WIFI_PASSWORD "08130103"           // Senha do Wi-Fi

// --------------------- CONFIGURAÇÕES FIREBASE ------------------
#define API_KEY "AIzaSyCD4SZgGuH-_85WjSvo3WhS9CIIFxhOX8E"
#define DATABASE_URL "https://esp-project-d4bbe-default-rtdb.firebaseio.com/"

// --------------------- CONFIGURAÇÕES GPS -----------------------
static const int RXPin = 4;    // RX do ESP32-C3 <- TX do GPS
static const int TXPin = 3;    // TX do ESP32-C3 -> RX do GPS
static const uint32_t GPSBaud = 9600;      // maioria dos módulos GPS usa 9600

// --------------------- OBJETOS GLOBAIS -------------------------
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool fixObtido = false;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\n[BOOT] ESP32-C3 com GPS + Firebase");

  SerialGPS.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);

  // --------------------- CONECTAR WI-FI ------------------------
  Serial.print("[WIFI] Conectando-se a ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n[WIFI] Conectado!");
  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.localIP());

  // --------------------- CONFIGURAR FIREBASE -------------------
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("[FIREBASE] Autenticação anônima OK!");
    signupOK = true;
  } else {
    Serial.print("[FIREBASE] Falha ao autenticar: ");
    Serial.println(config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Alimenta o parser do GPS
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());

    // Se a posição foi atualizada
    if (gps.location.isUpdated()) {
      if (gps.location.isValid()) {
        if (!fixObtido) {
          Serial.println("\n==============================");
          Serial.println("[GPS] FIX OBTIDO! 🎉");
          Serial.println("==============================\n");
          fixObtido = true;
        }

        Serial.print("Latitude : ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);

        Serial.print("Satélites: ");
        Serial.println(gps.satellites.value());

        Serial.print("Precisão HDOP: ");
        Serial.println(gps.hdop.hdop());

        Serial.println("----------------------");
      }
    }
  }

  // A cada 5 segundos, envia para o Firebase
  if (millis() - sendDataPrevMillis > 5000) {
    sendDataPrevMillis = millis();

    if (gps.location.isValid() && signupOK && Firebase.ready()) {
      float latitude  = gps.location.lat();
      float longitude = gps.location.lng();

      if (Firebase.RTDB.setFloat(&fbdo, "/gps/latitude", latitude)) {
        Serial.print("[FIREBASE] Latitude enviada: ");
        Serial.println(latitude);
      } else {
        Serial.print("[FIREBASE] Erro latitude: ");
        Serial.println(fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "/gps/longitude", longitude)) {
        Serial.print("[FIREBASE] Longitude enviada: ");
        Serial.println(longitude);
      } else {
        Serial.print("[FIREBASE] Erro longitude: ");
        Serial.println(fbdo.errorReason());
      }
    } else if (!gps.location.isValid()) {
      Serial.println("[GPS] Ainda aguardando fix...");
    }
  }
}

