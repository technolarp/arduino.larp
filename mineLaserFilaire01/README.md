# Mine laser et filaire n°01
Ce montage permet de simuler une mine laser ou filaire pour un jeu de role grandeur nature (jdr GN) ou une partie d'airsoft  
!! CE N'EST PAS UNE VERITABLE BOMBE !! 

## Matériel
Vous aurez besoin de
- une carte Arduino uno ou nano
- un écran OLED de type SSD1306 de taille 128x32 pixels
- un photoresistor et d'une résistance 10 kOhms
- une diode laser 5v
- des fils et un jumper (cavalier) type carte mere de PC
- Une bobine de fil de pêche transparent
- une alarme 5-12v et 1 mosfet 2N7000
- une LED neopixel ou ws2811
- une bouton poussoir NO (Normally Open)
- un jumper de carte mere de PC ou un interrupteur on/off


## Utilisation
1. Allumer la mine, une animation s'affiche sur l'écran OLED, puis la led clignote en vert lentement
1. Appuyer sur le bouton on/off pour choisir le mode de la mine (filaire ou laser)
1. Mode filaire
	1. Attacher le jumper à un fil de pêche et attacher le fil de pêche à un objet fixe
	1. Appuyer sur le bouton d'activation
1. Mode laser
	1. Aligner le laser et le photoresistor
	1. Appuyer sur le bouton d'activation
	1. La mine calibre le photoresistor pendant environ 5 secondes
1. La mine est activée et la led clignote lentement en rouge

Si quelqu'un tire sur le fil de pêche, le jumper est retiré et la mine "explose".
Si quelqu'un coupe le laser, la mine "explose". 
Dans les deux cas, la led clignote en rouge rapidement et l'alarme sonne pendant environ 5 secondes.  

Il est possible de désactiver une mine active grâce à un "cheat code". Voir plus bas

Une fois la mine "explosée", la led clignote en bleu lentement
Si le jumper de rearmement automatique est présent, la mine se réarme toute seule après explosion

## Bibliothèque
Vous aurez besoin d'installer les bibliothèques
1. [FastLED.h](https://github.com/FastLED/FastLED)  
1. [Adafruit_GFX.h](https://github.com/adafruit/Adafruit-GFX-Library)
1. [Adafruit_SSD1306.h](https://github.com/adafruit/Adafruit_SSD1306)

Remarque :  
Pour utiliser une led ws2811, décommenter la ligne  
```FastLED.addLeds<WS2811, LED_DATA_PIN>(leds, NB_LEDS);```  
Pour utiliser une led neopixel de chez Adafruit, décommenter la ligne  
```FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NB_LEDS);```  

## Durée de l'alarme
Il est possible de configurer la durée de l'alarme en cas d'explosion dans le code grâce à la définition DELAI_ALARME  
Par défaut le délai est de 5 secondes  
```c
// DELAI ALARME
#define DELAI_ALARME 5 // durée de l'alarme en seconde
```

## Cheat code
Il est possible de désactiver la mine en entrant une séquence à l'aide du bouton on/off de sélection et du bouton poussoir d'activation  
Par défaut, la séquence du cheatcode est 1 0 0 1  
  
Appuyer sur le bouton on/off de sélection du mode = 1  
Appuyer sur le bouton poussoir d'activation = 0  

Vous pouvez modifier la séquence et la taille du cheatcode dans le code arduino :
```c
// CHEATCODE
#define CHEATCODE_LENGTH 4
byte cheatCode[CHEATCODE_LENGTH] = {1,0,0,1};
```

## Exemples
Mine laser réalisée avec une carte Arduino Nano et un pcb de prototypage   
  
![](./images/mineLaserFilaire01_exemple01.png)  
La mine en mode filaire  

![](./images/mineLaserFilaire01_exemple02.png)  
La mine en mode laser    

![](./images/mineLaserFilaire01_exemple03.png)   
le laser réalisé à partir d'un bloc 3 piles AAA et un un support imprimé en PLA

J'ai utilisé une alarme 6-12V pour moto : https://www.aliexpress.com/item/32221510247.html  

## Câblage
Pin Arduino  | Composant | Commentaire
---------|------------|------------
A0 | PHOTORESISTOR_PIN | photoresistor résistance pulldown 10 kOhms
A4 | SDA | I²C SDA de l'écran OLED
A5 | SCL | I²C SCL de l'écran OLED
D2 | LED_DATA_PIN | led RGB neoPixel ou ws2811
D3 | BOUTON_ACTIVATION_PIN | bouton d'activation
D4 | MODE_PIN | interrupteur de sélection du mode filaire ou laser
D5 | FIL_PIEGE_PIN | fil piège
D6 | REARM_PIN | réarmement automatique de la mine
D7 | ALARME_PIN | mosfet et alarme

## Schema 
![](./images/mineLaserFilaire01_schema.png)

Les deux résistances et la diode sont là pour protéger le mosfet et la carte Arduino

## Alimentation
Il existe plusieurs façons d'alimenter une carte Arduino Uno.
[Quelques explications](https://www.open-electronics.org/the-power-of-arduino-this-unknown/)
1. câble USB, 5v
1. fiche DC 2.1mm, de 6-12v
1. pin 5v, 5v
1. pin Vin, de 6-12v
