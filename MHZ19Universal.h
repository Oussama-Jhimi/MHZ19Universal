/* MHZ19Universal.h  |  Version: 1.1.0  |  License: LGPLv3  */
#ifndef MHZ19UNIVERSAL_H
#define MHZ19UNIVERSAL_H

#include <Arduino.h>
#include <string.h> // memcpy

// Décommenter pour activer les logs de debug série
// #define MHZ19_DEBUG

#ifdef MHZ19_DEBUG
  #define MHZ19_DBG(x) Serial.print(x)
  #define MHZ19_DBLN(x) Serial.println(x)
#else
  #define MHZ19_DBG(x)
  #define MHZ19_DBLN(x)
#endif

#define MHZ19_DATA_LEN     9
#define TIMEOUT_MS         1000u  // délai d'attente pour la trame (ms)

// Error codes
enum MHZ19_Error : int {
  MHZ19_OK = 1,
  MHZ19_TIMEOUT,
  MHZ19_CRC_ERROR,
  MHZ19_MATCH_ERROR,
  MHZ19_INVALID_RESPONSE
};

// Supported sensor variants
enum MHZ19_Variant : uint8_t {
  MHZ19_UNKNOWN = 0,
  MHZ19_B,
  MHZ19_C,
  MHZ19_D
};

class MHZ19Universal {
public:
  MHZ19Universal();

  // Initialise avec n'importe quel Stream (HardwareSerial, SoftwareSerial, ...)
  void begin(Stream &serial);

  // Lectures
  MHZ19_Error readCO2(int &ppm);
  MHZ19_Error readTemperature(float &temp);

  // Calibration & configuration
  MHZ19_Error calibrateZero();                // calibrage à 400 ppm
  MHZ19_Error calibrateSpan(int ppm);         // calibrage span
  MHZ19_Error enableAutoCalibration(bool on); // active/désactive ABC
  MHZ19_Error setRange(int range);            // ex: 2000 ou 5000

  // (Optionnel) déverrouiller/verrouiller paramètres (certaines firmwares)
  MHZ19_Error unlock(uint16_t code); // envoyer un code d'unlock si nécessaire
  MHZ19_Error lock();                // verrouille (habituellement en envoyant 0)

  // Détection / info
  MHZ19_Variant detectVariant();
  String variantName() const;

  // Filtrage simple (moyenne mobile) : mettre 1 pour désactiver
  void setFilterWindow(uint8_t samples);

  // Récupérer la trame brute (9 octets) pour debug
  void getLastRawResponse(uint8_t *outBuffer) const;

private:
  Stream* _serial;
  uint8_t _buffer[MHZ19_DATA_LEN];
  MHZ19_Variant _variant;
  uint8_t _filterWindow;
  long _filterSum;
  uint8_t _filterCount;

  MHZ19_Error sendCommand(uint8_t cmd, uint16_t value = 0);
  MHZ19_Error receiveResponse();
  uint8_t     computeCRC(const uint8_t *buf) const;
  void        makeBytes(uint16_t value, uint8_t &high, uint8_t &low) const;
  int         toInt(uint8_t high, uint8_t low) const;
};

#endif // MHZ19UNIVERSAL_H
