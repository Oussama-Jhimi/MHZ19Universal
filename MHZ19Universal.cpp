/* MHZ19Universal.cpp  |  Version: 1.1.0  */
#include "MHZ19Universal.h"

// Command codes (partagés / communs)
static const uint8_t CMD_READ    = 0x86; // lecture CO2+temp
static const uint8_t CMD_ZERO    = 0x87; // calibre zéro (400ppm)
static const uint8_t CMD_SPAN    = 0x88; // calib span
static const uint8_t CMD_ABC     = 0x79; // auto calibration toggle
static const uint8_t CMD_RANGE   = 0x99; // changer range (2000/5000)
static const uint8_t CMD_VERSION = 0xA0; // commande de version/firmware
static const uint8_t CMD_UNLOCK  = 0x9F; // unlock/lock (varie selon firmwares) - valeur générique

MHZ19Universal::MHZ19Universal()
: _serial(nullptr),
  _variant(MHZ19_UNKNOWN),
  _filterWindow(1),
  _filterSum(0),
  _filterCount(0) {}

void MHZ19Universal::begin(Stream &serial) {
  _serial = &serial;
  // tentative de detection (non bloquante si capteur ne répond pas)
  _variant = detectVariant();
}

MHZ19_Variant MHZ19Universal::detectVariant() {
  if (!_serial) return MHZ19_UNKNOWN;
  if (sendCommand(CMD_VERSION) != MHZ19_OK) return MHZ19_UNKNOWN;
  if (receiveResponse() != MHZ19_OK) return MHZ19_UNKNOWN;

  // Certaines versions renvoient l'info dans différents octets.
  // On prend le plus grand des deux pour être tolérant.
  uint8_t a = _buffer[2];
  uint8_t b = _buffer[3];
  uint8_t v = (a >= b) ? a : b;

  MHZ19_DBG("detectVariant buf: ");
  for (uint8_t i=0;i<MHZ19_DATA_LEN;i++){ MHZ19_DBG(_buffer[i]); MHZ19_DBG(" "); }
  MHZ19_DBLN("");

  if (v >= 0x30) return MHZ19_D;
  if (v >= 0x14) return MHZ19_C; // seuil intermédiaire, adaptatif
  return MHZ19_B;
}

String MHZ19Universal::variantName() const {
  switch (_variant) {
    case MHZ19_B: return "MH-Z19B";
    case MHZ19_C: return "MH-Z19C";
    case MHZ19_D: return "MH-Z19D";
    default:      return "MH-Z19 (Unknown)";
  }
}

MHZ19_Error MHZ19Universal::readCO2(int &ppm) {
  if (!_serial) return MHZ19_INVALID_RESPONSE;
  if (sendCommand(CMD_READ) != MHZ19_OK) return MHZ19_INVALID_RESPONSE;
  if (receiveResponse() != MHZ19_OK) return MHZ19_CRC_ERROR;

  int raw = toInt(_buffer[2], _buffer[3]);

  if (_filterWindow > 1) {
    _filterSum += raw;
    _filterCount++;
    if (_filterCount >= _filterWindow) {
      ppm = (int)(_filterSum / _filterWindow);
      _filterSum = 0;
      _filterCount = 0;
    } else {
      // tant que la fenêtre n'est pas remplie, renvoyer la valeur brute
      ppm = raw;
    }
  } else {
    ppm = raw;
  }
  return MHZ19_OK;
}

MHZ19_Error MHZ19Universal::readTemperature(float &temp) {
  if (!_serial) return MHZ19_INVALID_RESPONSE;
  if (sendCommand(CMD_READ) != MHZ19_OK) return MHZ19_INVALID_RESPONSE;
  if (receiveResponse() != MHZ19_OK) return MHZ19_CRC_ERROR;

  // Beaucoup d'implémentations MH-Z19 utilisent: temp = buf[4] - 40
  temp = (float)_buffer[4] - 40.0f;
  return MHZ19_OK;
}

MHZ19_Error MHZ19Universal::calibrateZero() {
  return sendCommand(CMD_ZERO);
}

MHZ19_Error MHZ19Universal::calibrateSpan(int ppm) {
  uint8_t hi, lo;
  makeBytes((uint16_t)ppm, hi, lo);
  uint16_t v = ((uint16_t)hi << 8) | lo;
  return sendCommand(CMD_SPAN, v);
}

MHZ19_Error MHZ19Universal::enableAutoCalibration(bool on) {
  return sendCommand(CMD_ABC, on ? 0x00A0 : 0x0000);
}

MHZ19_Error MHZ19Universal::setRange(int range) {
  // validation basique, adapter selon ton module
  if (range != 2000 && range != 5000) return MHZ19_INVALID_RESPONSE;
  uint8_t hi, lo;
  makeBytes((uint16_t)range, hi, lo);
  uint16_t v = ((uint16_t)hi << 8) | lo;
  return sendCommand(CMD_RANGE, v);
}

MHZ19_Error MHZ19Universal::unlock(uint16_t code) {
  // Certains firmwares nécessitent une commande unlock avant modifications
  return sendCommand(CMD_UNLOCK, code);
}

MHZ19_Error MHZ19Universal::lock() {
  // verrouiller: envoyer 0 (selon firmware)
  return sendCommand(CMD_UNLOCK, 0x0000);
}

void MHZ19Universal::setFilterWindow(uint8_t samples) {
  _filterWindow = max((uint8_t)1, samples);
  _filterSum = 0;
  _filterCount = 0;
}

void MHZ19Universal::getLastRawResponse(uint8_t *outBuffer) const {
  if (!outBuffer) return;
  memcpy(outBuffer, _buffer, MHZ19_DATA_LEN);
}

/* ----------------- Bas niveau: envoi & réception ----------------- */

MHZ19_Error MHZ19Universal::sendCommand(uint8_t cmd, uint16_t value) {
  if (!_serial) return MHZ19_INVALID_RESPONSE;
  uint8_t packet[MHZ19_DATA_LEN] = {0xFF, 0x01, cmd, 0, 0, 0, 0, 0, 0};
  if (value) {
    packet[3] = (uint8_t)((value >> 8) & 0xFF);
    packet[4] = (uint8_t)(value & 0xFF);
  }
  packet[8] = computeCRC(packet);

  size_t written = _serial->write(packet, MHZ19_DATA_LEN);
  _serial->flush();
  delay(10); // court délai pour laisser le module traiter (sécurisé)
  if (written != MHZ19_DATA_LEN) {
    MHZ19_DBG("sendCommand: written != 9\n");
    return MHZ19_INVALID_RESPONSE;
  }
  return MHZ19_OK;
}

MHZ19_Error MHZ19Universal::receiveResponse() {
  if (!_serial) return MHZ19_INVALID_RESPONSE;
  unsigned long start = millis();
  uint8_t idx = 0;

  // Lecture jusqu'à trouver un octet 0xFF en début de trame puis lire le reste
  while (true) {
    // timeout
    if (millis() - start > TIMEOUT_MS) return MHZ19_TIMEOUT;

    if (_serial->available()) {
      uint8_t b = (uint8_t)_serial->read();
      // chercher le préfixe 0xFF
      if (idx == 0) {
        if (b != 0xFF) {
          // on ignore les bytes jusqu'au préfixe
          continue;
        }
      }
      _buffer[idx++] = b;
      // une fois reçu le premier octet, lire les 8 restants
      if (idx >= MHZ19_DATA_LEN) break;
    } else {
      // libère le CPU (utile sur ESP)
      yield();
    }
  }

  // Vérification CRC
  uint8_t crc = computeCRC(_buffer);
  if (_buffer[8] != crc) {
    MHZ19_DBG("receiveResponse: CRC mismatch\n");
    #ifdef MHZ19_DEBUG
      Serial.print("Buf: ");
      for (uint8_t i=0;i<MHZ19_DATA_LEN;i++) {
        Serial.print(_buffer[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    #endif
    return MHZ19_CRC_ERROR;
  }

  return MHZ19_OK;
}

uint8_t MHZ19Universal::computeCRC(const uint8_t *buf) const {
  uint16_t sum = 0;
  for (uint8_t i = 1; i < 8; i++) sum += buf[i];
  return (uint8_t)(255 - (sum % 256) + 1);
}

void MHZ19Universal::makeBytes(uint16_t value, uint8_t &high, uint8_t &low) const {
  high = (uint8_t)((value >> 8) & 0xFF);
  low  = (uint8_t)(value & 0xFF);
}

int MHZ19Universal::toInt(uint8_t high, uint8_t low) const {
  return ((int)high << 8) | (int)low;
}
