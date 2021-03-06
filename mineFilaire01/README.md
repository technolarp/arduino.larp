# Mine à fil n°01
Ce montage permet de simuler une mine filaire pour un jeu de rôle grandeur nature (jdr GN) ou une partie d'airsoft  
!! CE N'EST PAS UNE VERITABLE BOMBE !! 

## Matériel
Vous aurez besoin
- d'une carte Arduino uno ou nano
- des fils et un jumper (cavalier) type carte mere de PC
- Une bobine de fil de pêche transparent
- une alarme 5-12v et 1 mosfet 2N7000
- une LED neopixel ou ws2811
- une bouton poussoir NO (Normally Open)

## User manual
1. Allumer la mine, la led clignote en vert lentement
1. Connecter le jumper au fil piège et à la masse
1. Attacher le jumper à un fil de pêche et attacher le fil de pêche à un objet fixe
1. Appuyer sur le bouton d'activation, la mine est activée et la led clignote lentement en rouge

Si quelqu'un tire sur le fil de pêche, le jumper est retiré et la mine "explose". La led clignote en rouge rapidement et l'alarme sonne pendant environ 5 secondes.  
Il n'est pas possible de désactiver la mine, sauf en l'éteignant  

Une fois la mine "explosée", la led clignote en bleu lentement

## Bibliothèque
Vous aurez besoin d'installer la bibliothèque [FastLED.h](https://github.com/FastLED/FastLED)  

Remarque :  
Pour utiliser une led ws2811, décommenter la ligne  
```FastLED.addLeds<WS2811, LED_DATA_PIN>(leds, NB_LEDS);```  
Pour utiliser une led neopixel de chez Adafruit, décommenter la ligne  
```FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NB_LEDS);```  


## Exemples
<img src="../images/mineFilaire01_mine.png" width="600">
Mine filaire réalisée avec une carte Arduino Uno et un proto shield  

J'ai utilisé une alarme 6-12V pour moto : https://www.aliexpress.com/item/32221510247.html  



## Fil piège
![](../images/mineFilaire01_fil_piege.png)  
Deux exemples pour le fil piège  
1. Deux broches de type "Dupont" mâle et un fil avec 2 terminaux de type "Dupont" femelle
1. Du fil électrique rigide (0.22mm²) et un jumper de carte mère

## Câblage
Pin Arduino  | Composant | Commentaire
---------|------------|------------
D2 | LED_DATA_PIN | led RGB neoPixel ou ws2811
D3 | BOUTON_ACTIVATION_PIN | bouton d'activation
D5 | FIL_PIEGE_PIN | fil piège
D7 | ALARME_PIN | mosfet et alarme

## Schema 
![](../images/mineFilaire01_schema.png)

Les deux résistances et la diode sont là pour protéger le mosfet et la carte Arduino

## Alimentation
Il existe plusieurs façons d'alimenter une carte Arduino Uno.
[Quelques explications](https://www.open-electronics.org/the-power-of-arduino-this-unknown/)
1. câble USB, 5v
1. fiche DC 2.1mm, de 6-12v
1. pin 5v, 5v
1. pin Vin, de 6-12v
