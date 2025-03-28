#include <Arduino.h>

int pH = A2;
int samp = 5;
float dc_resol = 1024; // Résolution de l'ADC
int relayswitch = 8;

void setup() // Fonction d'initialisation
{
  pinMode(relayswitch, OUTPUT); // La broche 8 est définie comme sortie pour contrôler un relais
  Serial.begin(9600); // Initialisation de la communication série à 9600 bauds
  while (!Serial) {
    ; // Attendre l'initialisation de la connexion série (utile pour les cartes comme Arduino Leonardo)
  }
  Serial.println("Le programme est prêt. Envoi des données de pH...");
}

float ph(float pvolt) {
  return 7 + ((2.5 - pvolt) / 0.18); // Conversion tension -> pH
}

void loop() {
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

  delay(1000); // Attendre 1 seconde avant la prochaine mesure (vous pouvez ajuster).
}
