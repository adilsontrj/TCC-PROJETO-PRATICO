#include <TinyGPS++.h>
#include <HardwareSerial.h>

// --------------------- CONFIGURAÇÕES GPS -----------------------
static const int RXPin = 4;    // RX do ESP32-C3 <- TX do GPS
static const int TXPin = 3;    // TX do ESP32-C3 -> RX do GPS
static const uint32_t GPSBaud = 9600;  // maioria dos GPS usa 9600 baud

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

bool fixObtido = false;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\n[BOOT] Teste de GPS iniciado...");

  SerialGPS.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.println("[GPS] Esperando dados do módulo...");
}

void loop() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());

    // Sempre que tiver uma nova posição
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

  // Caso ainda não tenha fix
  if (!fixObtido && millis() % 5000 < 50) {
    Serial.println("[GPS] Ainda aguardando fix...");
  }
}
