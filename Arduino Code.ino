#include <Arduino.h>
#include <EEPROM.h>

int pH = A2;
int samp = 5;
float dc_resol = 1024; // Résolution de l'ADC
int relayswitch = 8;

// Variable de calibration (valeur par défaut)
float neutralVoltage = 2.5; // Tension pour pH 7 (initialement estimée)

bool pauseMeasurement = true; // Démarrage en mode pause (pas de mesures)

void setup() {
  pinMode(relayswitch, OUTPUT); // La broche 8 comme sortie pour contrôler un relais
  Serial.begin(9600);
  while (!Serial) {
    ; // Attendre l'initialisation de la connexion série
  }

  // Charger les paramètres de calibration depuis l'EEPROM
  neutralVoltage = EEPROM_readFloat(0); // Charger la tension pour pH 7

  Serial.println("Le programme est prêt.");
  Serial.println("Envoyez 'start' pour commencer les mesures.");
  Serial.println("Envoyez 'calibration' pour calibrer.");
  Serial.println("Envoyez 'pause' pour arrêter les mesures.");
  Serial.println("Envoyez 'resume' pour reprendre les mesures.");
}

float ph(float pvolt) {
  float slope = 0.18;                          // Pente préestimée pour le capteur
  return 7 + ((neutralVoltage - pvolt) / slope); // Conversion tension -> pH
}

// Fonction de calibration
void calibrateSensor() {
  Serial.println("Calibration en cours...");
  Serial.println("Placez le capteur dans une solution de pH 7 et tapez 'OK'.");
  while (Serial.available() == 0);
  Serial.read(); // Attendre la confirmation de l'utilisateur (via 'OK')
  delay(2000); // Attendre une stabilisation

  // Mesurer la tension pour pH 7
  int measures = 0;
  for (int i = 0; i < samp; i++) {
    measures += analogRead(pH);
    delay(5);
  }
  neutralVoltage = 5.0 / dc_resol * measures / samp;
  EEPROM_writeFloat(0, neutralVoltage); // Sauvegarder dans l'EEPROM
  Serial.print("Tension pour pH 7 enregistrée : ");
  Serial.println(neutralVoltage);

  Serial.println("Calibration terminée !");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readString();
    input.trim();
    if (input.equalsIgnoreCase("calibration")) {
      calibrateSensor();
    } else if (input.equalsIgnoreCase("pause")) {
      pauseMeasurement = true;
      Serial.println("Mesures arrêtées.");
    } else if (input.equalsIgnoreCase("resume")) {
      pauseMeasurement = false;
      Serial.println("Mesures reprises.");
    } else if (input.equalsIgnoreCase("start")) {
      pauseMeasurement = false;
      Serial.println("Mesures démarrées.");
    }
  }

  // Si les mesures sont en pause, ignorer le reste de la boucle
  if (pauseMeasurement) {
    return;
  }

  // Mesure de la tension en moyenne
  int measures = 0;
  for (int i = 0; i < samp; i++) {
    measures += analogRead(pH);
    delay(5);
  }
  float volt = 5.0 / dc_resol * measures / samp;

  // Calcul du pH
  float pHValue = ph(volt);

  // Affichage sur le Moniteur Série
  Serial.print("pH Mesuré : ");
  Serial.println(pHValue);

  // Contrôle du relais en fonction du pH
  if (pHValue > 8.3) {
    digitalWrite(relayswitch, HIGH); // Activer le relais si pH > 8.3
  } else if (pHValue < 7.0) {
    digitalWrite(relayswitch, LOW); // Désactiver le relais si pH < 7.0
  }

  delay(1000); // Attendre 1 seconde avant la prochaine mesure
}

// Fonctions pour lire/écrire des floats dans l'EEPROM
void EEPROM_writeFloat(int addr, float data) {
  byte* ptr = (byte*)(void*)&data; // Pointer vers les octets dans le float
  for (int i = 0; i < sizeof(data); i++) {
    EEPROM.write(addr + i, *ptr++);
  }
}

float EEPROM_readFloat(int addr) {
  float data;
  byte* ptr = (byte*)(void*)&data; // Pointer vers les octets dans le float
  for (int i = 0; i < sizeof(data); i++) {
    *ptr++ = EEPROM.read(addr + i);
  }
  return data;
}
