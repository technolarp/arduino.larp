# Bombe arduino n°01
Ce montage permet de simuler une bombe pour un jeu de rôle grandeur nature (jdr GN) ou une partie d'airsoft  
!! CE N'EST PAS UNE VERITABLE BOMBE !! 

## Matériel
Vous aurez besoin de
- une carte Arduino uno ou nano
- un afficheur 4*7segment avec un contrôleur TM1637. [Ce modèle](https://robotdyn.com/4-digit-led-display-tube-7-segments-tm1637-50x19mm-red-double-dots-clock.html) fonctionne bien
- une LED rouge 5v et une résistance appropriée (220 ohms)
- un buzzer piezo 5v
- trois boutons poussoir NO (Normally Open)


## Utilisation
1. Allumer la bombe, l'afficheur indique 05:00
1. Appuyer sur le bouton haut ou bas pour régler le compte à rebours
1. Appuyer sur le bouton d'activation
1. La bombe est activée et le temps décompte sur l'afficheur

A la fin du compte à rebours, la bombe "explose". Le buzzer sonne pendant environ 5 secondes et l'afficheur clignote avec un "b o o n".  
Il n'est pas possible de désactiver la bombe, sauf en l'éteignant  


## Bibliothèque
Vous aurez besoin d'installer la bibliothèque [TM1637Display.h](https://github.com/avishorp/TM1637)  

Cette library est disponible via l'IDE arduino dans le library manager. menu Sketch->Include Library->Manage Libraries... Chercher la library "tm1637 by Avishay Orpaz". 
Cette library est aussi disponible sur [github](https://github.com/avishorp/TM1637) 

## Exemples
![](./images/bombe01_exemple01.png)  
Prototype avec un Arduino Nano (le connecteur JST blanc n'est pas utilisé)
  
![](./images/bombe01_exemple02.png)  
Arduino Uno et un [shield tick tock de chez Seedstudio](https://www.seeedstudio.com/Starter-Shield-EN-Tick-Tock-shield-v2-p-2768.html)  


## Câblage
Pin Arduino  | Composant | Commentaire
---------|------------|------------
D5 |   LED_ROUGE_PIN |   led rouge + resistance
D6 |   ALARME_PIN  |   buzzer piezo 5v
D7 |   CLK_PIN |   TM1637 pin CLK
D8 |   DIO_PIN |   TM1637 pin DIO
D9 |   BOUTON_BAS_PIN |  bouton haut
D10 |  BOUTON_HAUT_PIN | bouton bas
D11 |  BOUTON_ACTIVATION_PIN | bouton d'activation
 

## Schema 
![](./images/bombe01_schema.png)


## Alimentation
Il existe plusieurs façons d'alimenter une carte Arduino Uno.
[Quelques explications](https://www.open-electronics.org/the-power-of-arduino-this-unknown/)
1. câble USB, 5v
1. fiche DC 2.1mm, de 6-12v
1. pin 5v, 5v
1. pin Vin, de 6-12v
