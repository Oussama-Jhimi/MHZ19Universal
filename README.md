# MHZ19Universal

Bibliothèque Arduino/Embedded pour capteurs MH-Z19 (B/C/D) — version 1.1.0

## Objectif
Fournir une API unifiée, robuste et portable pour lire la concentration CO2 et la température depuis
capteurs MH-Z19B, MH-Z19C, MH-Z19D. Inclus: détection de variante, calibration, auto-calibration,
changement de plage et filtrage logiciel (moyenne mobile).

## Contenu
- `MHZ19Universal.h` / `MHZ19Universal.cpp` : bibliothèque
- `examples/ExempleDeBase/ExempleDeBase.ino` : sketch d'exemple
- `README.md` : ce fichier
- `library.properties` : métadonnées pour l'IDE Arduino
- `keywords.txt` : mots-clés pour l'IDE

## Installation
1. Extraire le dossier `MHZ19Universal` dans `Documents/Arduino/libraries/` (Windows) ou `~/Arduino/libraries/`.
2. Redémarrer l'IDE Arduino.
3. Ouvrir `Fichier > Exemples > MHZ19Universal > ExempleDeBase`.

## Utilisation de base
- Inclure la bibliothèque:
  ```cpp
  #include <MHZ19Universal.h>
  ```
- Initialiser (ex. SoftwareSerial):
  ```cpp
  SoftwareSerial sw(10, 11);
  MHZ19Universal sensor;
  sensor.begin(sw);
  ```
- Lire CO2:
  ```cpp
  int co2;
  if (sensor.readCO2(co2) == MHZ19_OK) {
    Serial.println(co2);
  }
  ```
- Lire température:
  ```cpp
  float temp;
  sensor.readTemperature(temp);
  ```

## Notes importantes
- **Tension logique**: la plupart des modules MH-Z19 utilisent TTL 5V. Si vous utilisez un ESP (3.3V), adaptez le niveau logique.
- **Offset température**: la bibliothèque utilise `temp = buf[4] - 40`. Certains clones sortent un autre offset (ex: -2). Vérifiez en comparant avec un thermomètre et ajustez si besoin.
- **Commande version / détectation**: la détection automatique est tolérante (teste plusieurs octets). Si votre module ne répond pas à `CMD_VERSION`, la détection renverra `Unknown`.
- **Unlock/lock**: certains firmwares requièrent un déverrouillage avant de changer des paramètres. Le code `0xA5A5` dans l'exemple est fictif — renseignez-vous sur votre module.

## Debug
- Pour activer les logs série détaillés, décommentez `#define MHZ19_DEBUG` en haut de `MHZ19Universal.h`.
- Vous pouvez récupérer la dernière trame brute avec `getLastRawResponse()`.

## Changelog
- v1.1.0: Détection tolérante, debug optionnel, getLastRawResponse, amélioration receiveResponse, docs.

## Licence
LGPLv3 — voir les fichiers de licence officiels si vous publiez ce code.

