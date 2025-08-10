#include <MHZ19Universal.h>
#include <SoftwareSerial.h>

// Debug: décommenter dans MHZ19Universal.h si tu veux des logs
// #define MHZ19_DEBUG

SoftwareSerial sw(10, 11); // RX (D10) <= TX capteur, TX (D11) => RX capteur
MHZ19Universal sensor;

void setup() {
  Serial.begin(115200);
  sw.begin(9600);
  sensor.begin(sw);

  // Exemple : déverrouiller (si nécessaire), définir range, puis reverrouiller
  sensor.unlock(0xA5A5); // attention: le code dépend du firmware (exemple générique)
  sensor.setRange(5000);
  sensor.lock();

  sensor.setFilterWindow(5);     // moyenne glissante sur 5 échantillons
  sensor.enableAutoCalibration(false);
  delay(200);

  Serial.println("MH-Z19Universal Example");
  Serial.println("Sensor variant: " + sensor.variantName());
}

void loop() {
  int co2 = 0;
  float temp = 0.0f;
  if (sensor.readCO2(co2) == MHZ19_OK) {
    sensor.readTemperature(temp);
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm  |  Temp: ");
    Serial.print(temp);
    Serial.println(" °C");
  } else {
    Serial.println("Erreur de lecture capteur");
  }
  delay(2000);
}
