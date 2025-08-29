#include <WiFi.h>
#include <FirebaseESP32.h>
#include <TinyGPS++.h>

// Configuração Wi-Fi
#define WIFI_SSID "Rep.Alcooltece5G"
#define WIFI_PASSWORD "08130103"

// Configuração Firebase
#define FIREBASE_HOST "https://esp-project-d4bbe-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyCD4SZgGuH-_85WjSvo3WhS9CIIFxhOX8E"

// GPS
TinyGPSPlus gps;
#define RXD2 21
#define TXD2 20
HardwareSerial SerialGPS(1);

// Inicializa Firebase
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Conecta no Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Inicializa Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());

    if (gps.location.isUpdated()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      Serial.print("Lat: ");
      Serial.print(latitude, 6);
      Serial.print(" | Lng: ");
      Serial.println(longitude, 6);

      // Envia para Firebase
      if (Firebase.RTDB.setFloat(&firebaseData, "/gps/latitude", latitude)) {
        Serial.println("Latitude enviada!");
      } else {
        Serial.print("Erro Latitude: ");
        Serial.println(firebaseData.errorReason());
      }

      if (Firebase.RTDB.setFloat(&firebaseData, "/gps/longitude", longitude)) {
        Serial.println("Longitude enviada!");
      } else {
        Serial.print("Erro Longitude: ");
        Serial.println(firebaseData.errorReason());
      }

      delay(2000); // Intervalo entre envios
    }
  }
}
