#include <Arduino.h>
#include <EEPROM.h>// Définition des broches
int pH = A2;              // Broche analogique connectée au capteur de pH
int samp = 5;             // Nombre d'échantillons pour le lissage de la lecture du pH
float dc_resol = 1024;    // Résolution de l'ADC (Analog to Digital Converter) de l'Arduino (10 bits)
int relayswitch = 8;       // Broche digitale connectée au relais

// Variable de calibration (valeur par défaut)
float neutralVoltage = 2.5; // Tension pour pH 7 (initialement estimée).  Cette valeur sera ajustée via la calibration.

// Variable de contrôle de l'état des mesures
bool pauseMeasurement = true; // Démarrage en mode pause (pas de mesures). Permet de contrôler si les mesures sont actives ou non.

void setup() {
  // Initialisation des broches
  pinMode(relayswitch, OUTPUT); // La broche du relais est configurée comme une sortie.

  // Initialisation de la communication série
  Serial.begin(9600); // Initialise la communication série à 9600 bauds.
  while (!Serial) {
    ; // Attendre l'initialisation de la connexion série (utile sur certaines cartes Arduino).
  }

  // Lecture des paramètres de calibration depuis l'EEPROM
  neutralVoltage = EEPROM_readFloat(0); // Charge la tension pour pH 7 depuis l'adresse 0 de l'EEPROM.  Si aucune valeur n'est stockée, la valeur par défaut sera utilisée lors de la première exécution.

  // Information à l'utilisateur
  Serial.println("Le programme est prêt.");
  Serial.println("Envoyez 'start' pour commencer les mesures.");
  Serial.println("Envoyez 'calibration' pour calibrer.");
  Serial.println("Envoyez 'pause' pour arrêter les mesures.");
  Serial.println("Envoyez 'resume' pour reprendre les mesures.");
}

// Fonction de conversion tension -> pH
float ph(float pvolt) {
  float slope = 0.18;                          // Pente préestimée pour le capteur.  Cette valeur peut nécessiter un ajustement en fonction du capteur utilisé.  Elle représente la variation de tension par unité de pH.
  return 7 + ((neutralVoltage - pvolt) / slope); // Conversion tension -> pH.  Formule de conversion basée sur la tension mesurée, la tension de calibration pour pH 7 et la pente du capteur.
}

// Fonction de calibration du capteur
void calibrateSensor() {
  Serial.println("Calibration en cours...");
  Serial.println("Placez le capteur dans une solution de pH 7 et tapez 'OK'.");
  while (Serial.available() == 0); // Attend que l'utilisateur entre quelque chose dans le moniteur série.
  Serial.read(); // Lit la confirmation de l'utilisateur (via 'OK') et vide le buffer série.  La valeur lue n'est pas utilisée, on attend juste une entrée.
  delay(2000); // Attend 2 secondes pour permettre à la lecture du capteur de se stabiliser.

  // Mesure de la tension pour pH 7
  int measures = 0;
  for (int i = 0; i < samp; i++) {
    measures += analogRead(pH); // Lit la valeur analogique du capteur de pH et l'ajoute à la somme.
    delay(5); // Petit délai entre les lectures pour améliorer la précision.
  }
  neutralVoltage = 5.0 / dc_resol * measures / samp; // Calcule la tension moyenne à partir des 'samp' lectures.  5.0 est la tension de référence de l'Arduino (VCC).
  EEPROM_writeFloat(0, neutralVoltage); // Sauvegarde la tension mesurée dans l'EEPROM à l'adresse 0.  Ainsi, la valeur est conservée même après une coupure de courant.
  Serial.print("Tension pour pH 7 enregistrée : ");
  Serial.println(neutralVoltage);

  Serial.println("Calibration terminée !");
}

void loop() {
  // Gestion des commandes série
  if (Serial.available()) {
    String input = Serial.readString(); // Lit la chaîne de caractères envoyée par le moniteur série.
    input.trim(); // Supprime les espaces blancs au début et à la fin de la chaîne.
    if (input.equalsIgnoreCase("calibration")) {
      calibrateSensor(); // Démarre la procédure de calibration.
    } else if (input.equalsIgnoreCase("pause")) {
      pauseMeasurement = true; // Met les mesures en pause.
      Serial.println("Mesures arrêtées.");
    } else if (input.equalsIgnoreCase("resume")) {
      pauseMeasurement = false; // Reprend les mesures.
      Serial.println("Mesures reprises.");
    } else if (input.equalsIgnoreCase("start")) {
      pauseMeasurement = false; // Démarre les mesures (équivalent à resume si le programme était en pause au démarrage).
      Serial.println("Mesures démarrées.");
    }
  }

  // Si les mesures sont en pause, on sort de la boucle loop pour ne pas effectuer de mesures
  if (pauseMeasurement) {
    return; // Retourne au début de la boucle loop sans exécuter le reste du code.
  }

  // Mesure de la tension en moyenne
  int measures = 0;
  for (int i = 0; i < samp; i++) {
    measures += analogRead(pH); // Lit la valeur analogique du capteur de pH et l'ajoute à la somme.
    delay(5); // Petit délai entre les lectures pour améliorer la précision.
  }  float volt = 5.0 / dc_resol * measures / samp; // Calcule la tension moyenne à partir des 'samp' lectures.

  // Calcul du pH
  float pHValue = ph(volt); // Convertit la tension mesurée en pH en utilisant la fonction 'ph'.

  // Affichage sur le Moniteur Série
  Serial.print("pH Mesuré : ");
  Serial.println(pHValue);

  // Contrôle du relais en fonction du pH
  if (pHValue > 8.3) {
    digitalWrite(relayswitch, HIGH); // Active le relais si le pH est supérieur à 8.3.
  } else if (pHValue < 7.0) {
    digitalWrite(relayswitch, LOW); // Désactive le relais si le pH est inférieur à 7.0.
  }

  delay(1000); // Attend 1 seconde avant la prochaine mesure.
}

// Fonctions pour lire/écrire des floats dans l'EEPROM
// L'EEPROM ne peut stocker que des octets, donc on utilise des pointeurs pour convertir le float en une série d'octets.
void EEPROM_writeFloat(int addr, float data) {
  byte* ptr = (byte*)(void*)&data; // Crée un pointeur 'ptr' qui pointe vers l'adresse mémoire de la variable 'data' (de type float) et la considère comme un tableau d'octets.
  for (int i = 0; i < sizeof(data); i++) {
    EEPROM.write(addr + i, *ptr++); // Écrit chaque octet du float dans l'EEPROM, en incrémentant l'adresse à chaque octet.
  }
}

float EEPROM_readFloat(int addr) {
  float data;
  byte* ptr = (byte*)(void*)&data; // Crée un pointeur 'ptr' qui pointe vers l'adresse mémoire de la variable 'data' (de type float) et la considère comme un tableau d'octets.  for (int i = 0; i < sizeof(data); i++) {
    *ptr++ = EEPROM.read(addr + i); // Lit chaque octet depuis l'EEPROM et le place dans la variable 'data', en incrémentant l'adresse à chaque octet.  }
  return data; // Retourne la valeur du float reconstituée à partir des octets lus dans l'EEPROM.
}
