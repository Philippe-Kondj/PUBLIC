#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
int pH = A2;
int samp = 5;
float dc_resol = 1024;
int relayswitch = 10; 
const int chips = 2;
void setup() // fonction de configuration
{
pinMode(10, OUTPUT); // la broche 10 est définie comme sortie pour contrôler un relais
Serial.begin(9600); // La communication série est initialisée avec un débit de 9600 bits par seconde
Serial.println("CF");
Serial.begin(9600);
while (!Serial) {;
}
Serial.print("Initialize");
if (!SD.begin(chips)) // La carte SD est initialisée sur le bus SPI, en vérifiant si elle est correctement montée
{
Serial.println("CF");
while (1); // Si l'initialisation échoue, le programme affiche "CF" sur le moniteur série et reste en boucle
}
Serial.println("sdinitiated.");
}
float ph (float pvolt) { // La fonction ph(float pvolt) calcule la valeur du pH en se basant sur la tension mesurée dans le capteur
return 7 + ((2.5 - pvolt) / 0.18); // La formule utilisée suppose une tension de référence médiane de 2.5 V pour un pH neutre de 7 - Le facteur de correction pour le pH est basé sur la sensibilité du capteur (0.18 dans ce cas)
}
void loop() // Dans la boucle principale (loop()), on mesure la tension moyenne plusieurs fois pour une meilleure précision.
{
int measures=0;
for (int i = 0; i < samp; i++)
{
measures += analogRead(pH);
delay(5);
}
float volt = 5 / dc_resol * measures/samp; // formule calculant la tension présente, avec : 5 représente la tension d'alimentation - dc_resol correspond à la résolution de l'ADC (convertisseur analogique-numérique) de l’Arduino, qui est de 1024 pour une tension de 5 V - samp signifie le nombre de mesures prises (dans ce cas, 5).
Serial.print("pHval");
Serial.println(ph(volt));
delay(600000);
if (ph(volt)>8.3)
{
digitalWrite(8,HIGH); // Si le pH dépasse 8.3 (pH alcalin), la broche 8 est activée (HIGH)
}
else if (ph(volt)<7.0) // Si le pH est inférieur à 7.0 (pH acide), la broche 8 est désactivée (LOW).
{
digitalWrite(8,LOW);
}String dataString = "pH";
for (int AP = A0; AP < 3; AP++) {
int sensor = analogRead(A2);
dataString += String(ph(volt));
if (AP > 2) {
dataString += "(ph(volt))";
}}
}
File DF = SD.open("DFlog.txt", FILE_WRITE); // Le programme ouvre le fichier texte en mode écriture avec SD.open("DFlog.txt", FILE_WRITE).
if (DF) {
DF.println(ph(volt));
DF.close(); // La méthode close() est appelée pour fermer le fichier et éviter des pertes de données ou une corruption
// print to the serial port too:
Serial.println(ph(volt)); // Le programme affiche sur le moniteur série (via Serial.println) : Le pH calculé. - Les messages d’état (comme "sdinitiated").
}
}