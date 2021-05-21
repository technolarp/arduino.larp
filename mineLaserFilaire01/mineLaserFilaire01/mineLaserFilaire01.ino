/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <technoLARP@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Marcelin
 * ----------------------------------------------------------------------------
 */

/*
 * ----------------------------------------------------------------------------
 * Ce montage permet de simuler une bombe/mine laser pour un jeu de role 
 * grandeur nature ou une partie d'airsoft
 * !! CE N'EST PAS UNE VERITABLE BOMBE !! 
 * 
 * Vous aurez besoin
 * une carte Arduino uno ou nano
 * un photoresistor et d'une résistance 10 kOhms
 * une diode laser 5v
 * des fils et un jumper (cavalier) type carte mere de PC
 * une alarme 5-12v et un mosfet 2N7000
 * une LED neopixel ou ws2811
 * une bouton poussoir NO (Normally Open)
 * un interrupteur on/off
 * un jumper de carte mere de PC ou un interrupteur on/off
 * un ecran OLED SSD1306 128x32
 * 
 * Vous aurez besoin d'installer les bibliothèques 
 * FastLED.h
 * Adafruit_GFX.h
 * Adafruit_SSD1306.h
 * ----------------------------------------------------------------------------
 */
 
/*
 * ----------------------------------------------------------------------------
 * PINOUT
 * A0     PHOTORESISTOR_PIN       photoresistor
 * D2     LED_DATA_PIN            led RGB neoPixel ou ws2811
 * D3     BOUTON_ACTIVATION_PIN   bouton d'activation
 * D4     MODE_PIN      		      interrupteur choix du mode laser ou filaire
 * D5     FIL_PIEGE_PIN           fil piege
 * D6     REARM_PIN 6             jumper/interrupteur pour rearmement
 * D7     ALARME_PIN              mosfet + alarme
 * A4     SDA                     pin SDA du bus I²C
 * A5     SCL                     pin SCL du bus I²C
 * 
 * ----------------------------------------------------------------------------
 */


// LED RGB
#include <FastLED.h>  // https://github.com/FastLED/FastLED

#define NB_LEDS 1
#define LED_DATA_PIN 2

// tableau de leds neopixel
CRGB leds[NB_LEDS];

// COMMANDES & LASER
#define PHOTORESISTOR_PIN A0
#define BOUTON_ACTIVATION_PIN 3
#define MODE_PIN 4
#define FIL_PIEGE_PIN 5
#define REARM_PIN 6
#define ALARME_PIN 7

int laserValeurReference;
int laserMarge = 15;
int laserValeurActuelle;

// STATUT DE LA MINE
enum {MINE_ALLUMEE, MINE_LASER_CALIBRAGE, MINE_LASER_ACTIVE, MINE_FIL_PIEGE_ACTIVE, MINE_EXPLOSEE};
byte statutMine = MINE_ALLUMEE;

// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>       // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>   // https://github.com/adafruit/Adafruit_SSD1306

#define OLED_LARGEUR 128 // largeur de l'ecran OLED, en pixels
#define OLED_HAUTEUR 32 // hauteur de l'ecran OLED, en pixels

// declaration d'un ecran OLED
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_LARGEUR, OLED_HAUTEUR, &Wire, OLED_RESET);

// DELAI ALARME
#define DELAI_ALARME 5 // durée de l'alarme en seconde
unsigned long int previousMillisAlarme;
unsigned long int currentMillisAlarme;
unsigned long int intervalAlarme;

// HEARTBEAT
unsigned long int previousMillisHB;
unsigned long int currentMillisHB;
unsigned long int intervalHB;

// DIVERS
bool animationUneFois = true;
bool appuiLong = false;

// CHEATCODE
#define TAILLE_CHEATCODE 4
byte sequenceCheatCode[TAILLE_CHEATCODE] = {1,0,0,1};
byte cheatCodeActuel[TAILLE_CHEATCODE];
bool statutInterrupteurPrecedent;

/*
 * ----------------------------------------------------------------------------
 * SETUP
 * ----------------------------------------------------------------------------
 */
void setup() 
{
  Serial.begin(115200);

  // LED RGB
  // pour utiliser une led ws2811
  //FastLED.addLeds<WS2811, LED_DATA_PIN>(leds, NB_LEDS);
  // pour utiliser une led adafruit neopixel
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NB_LEDS);
  FastLED.setBrightness(50);

  // COMMANDES & LASER
  pinMode(PHOTORESISTOR_PIN, INPUT);

  pinMode(FIL_PIEGE_PIN, INPUT);
  digitalWrite(FIL_PIEGE_PIN, HIGH);
  
  pinMode(BOUTON_ACTIVATION_PIN, INPUT);
  digitalWrite(BOUTON_ACTIVATION_PIN, HIGH);

  pinMode(MODE_PIN, INPUT);
  digitalWrite(MODE_PIN, HIGH);

  pinMode(ALARME_PIN, OUTPUT);
  digitalWrite(ALARME_PIN, LOW);

  // REARMEMENT
  pinMode(REARM_PIN, INPUT);
  digitalWrite(REARM_PIN, HIGH);
  
  // INITIALISER cheatCodeActuel
  for (int i=0;i<TAILLE_CHEATCODE;i++)
  {
	cheatCodeActuel[i] = 9;
  }

  // OLED
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))  // addresse i2c : 0x3C
  {
    Serial.println(F("SSD1306 erreur"));
    for(;;); // bloquer le programme
  }

  // texte en blanc
  display.setTextColor(SSD1306_WHITE);

  // animation de mise en route
  afficheText("INIT MINE", 2, true, true);
  for (int i=0;i<OLED_LARGEUR;i++)
  {
    display.fillRect(0,20,i,15,SSD1306_WHITE);
    display.display();
    delay(10);
  }
  
  delay(200);

  // DELAI ALARME
  intervalAlarme = DELAI_ALARME * 1000; 

  // HEARTBEAT
  previousMillisHB = millis();
  currentMillisHB = previousMillisHB;
  intervalHB = 1000;
  
  // DIVERS
  animationUneFois = true;
  display.clearDisplay();

  Serial.println("START !!!");
}
/*
 * ----------------------------------------------------------------------------
 * FIN DU SETUP
 * ----------------------------------------------------------------------------
 */




/*
 * ----------------------------------------------------------------------------
 * LOOP
 * ----------------------------------------------------------------------------
 */
void loop() 
{
  // loop en fonction du statut de la mine
  switch (statutMine) 
  {
    case MINE_ALLUMEE:
      // attendre l'activation
      mineAllumee();
    break;

    case MINE_LASER_CALIBRAGE:
      // calibrer la luminosité sur le photoresistor
      laser_calibrage();
    break;

    case MINE_LASER_ACTIVE:
      // le laser est en position, la mine est active
      mineLaserActive();
    break;

    case MINE_FIL_PIEGE_ACTIVE:
      // le fil piege de la mine est actif
      mineFilPiegeActive();
    break;
    
    case MINE_EXPLOSEE:
      // animation d'explosion
      mineExplosee();
    break;
    
    default:
      // rien a faire      
    break;
  }
}
/*
 * ----------------------------------------------------------------------------
 * FIN DU LOOP
 * ----------------------------------------------------------------------------
 */




/*
 * ----------------------------------------------------------------------------
 * FONCTIONS ADDITIONNELLES
 * ----------------------------------------------------------------------------
 */

// attendre l'activation
void mineAllumee()
{
  // heartbeat
  currentMillisHB = millis();
  if(currentMillisHB - previousMillisHB > intervalHB)
  {
    previousMillisHB = currentMillisHB;

    if (digitalRead(MODE_PIN))
    {
      clignoteLed(0,CRGB::Green, 1);
      
      char pourcent[6];
      sprintf(pourcent, "%d %%", (map(lirePhotoresistor(5),0,1024,0,100)));
      afficheText("MINE LASER", 1, true, false);
      afficheText(pourcent, 2, false, true);
    }
    else
    {
      clignoteLed(0,CRGB::Green, 2);
      afficheText("MINE FILAIRE", 1, true, false);

      if (digitalRead(FIL_PIEGE_PIN)==HIGH)
      {
        afficheText("DANGER !!", 2, false, true);
      }
      else
      {
        afficheText("PRETE", 2, false, true);
      }
    }
  }

  // detection de l'activation par le bouton poussoir
  if (appuiSurBouton(BOUTON_ACTIVATION_PIN))
  {
    if (digitalRead(MODE_PIN))
    {
      statutMine = MINE_LASER_CALIBRAGE;
      statutInterrupteurPrecedent = 1;
      Serial.println("ACTIVATION DU MODE LASER");

      for (int i=0;i<5;i++)
      {
        afficheText("LASER", 2, true, true);
        delay(200);
        afficheText("", 2, true, true);
        delay(200);
      }
    }
    else
    {
      statutMine = MINE_FIL_PIEGE_ACTIVE;
      statutInterrupteurPrecedent = 0;
      Serial.println("ACTIVATION DU MODE FIL PIEGE");

      for (int i=0;i<5;i++)
      {
        afficheText("FILAIRE", 2, true, true);
        delay(200);
        afficheText("", 2, true, true);
        delay(200);
      }
    }
  }
}


// calculer la luminosité sur le photoresistor
void laser_calibrage()
{
  laserValeurReference = 0;
  laserValeurActuelle = 0;

  Serial.println("CALIBRAGE");
  afficheText("CALIBRAGE", 2, true, false);
  
  for (int i=0;i<20;i++)
  {
    laserValeurActuelle+=lirePhotoresistor(5);
    
    // animation de calibrage    
    display.fillRect(0,20,(i*OLED_LARGEUR/20),15,SSD1306_WHITE);
    display.display();
    clignoteLed(0, CRGB::Orange, 1);
  }

  laserValeurActuelle/=20;
  laserValeurReference = laserValeurActuelle;
  Serial.print("laserValeurReference: ");
  Serial.println(laserValeurReference);

  // vider l'ecran oled
  display.clearDisplay();
  display.display();

  // activer la mine laser
  statutMine = MINE_LASER_ACTIVE;
}


// la mine laser est activee
void mineLaserActive()
{
  // mesurer le laser sur le photoresistor
  laserValeurActuelle = lirePhotoresistor(5);
      
  // check si le laser est coupé
  int margeActuelle = (float) (laserValeurReference-laserValeurActuelle) / (float) laserValeurReference * 100;
  
  if (margeActuelle >= laserMarge)
  {
    // le laser est coupé, la mine explose
    statutMine = MINE_EXPLOSEE;
  }

  // heartbeat
  currentMillisHB = millis();
  if(currentMillisHB - previousMillisHB > intervalHB)
  {
    previousMillisHB = currentMillisHB;

    // faire clignoter l'ecran OLED
    afficheText("LASER", 2, true, false);
    afficheText("ACTIF", 2, false, true);
    
    // faire clignoter la led
    clignoteLed(0, CRGB::Red, 1);
    afficheText("", 2, true, true);

    // sortie valeur actuelle
    Serial.println(laserValeurActuelle);
  }

  // gerer le cheatcode
  manageCheatcode();
}


// la mine filaire est activee
void mineFilPiegeActive()
{
  // check sur le fil piege
  int compteurDelai = 0;

  if (digitalRead(FIL_PIEGE_PIN)==HIGH)
  {
    delay(10);
    while( (digitalRead(FIL_PIEGE_PIN)==HIGH) && (compteurDelai<5) )
    {
      // debounce
      delay(50);
      compteurDelai++;
    }
  }

  if (compteurDelai == 5)
  {
    // le fil piege a ete retire, faire exploser la mine
    statutMine = MINE_EXPLOSEE;
  }

  // heartbeat
  currentMillisHB = millis();
  if(currentMillisHB - previousMillisHB > intervalHB)
  {
    previousMillisHB = currentMillisHB;
    
    // faire clignoter l'ecran OLED
    afficheText("FIL ACTIF", 2, true, true);
    
    // faire clignoter la led
    clignoteLed(0, CRGB::Red, 1);
    afficheText("", 2, true, true);
  }

  // gerer le cheatcode
  manageCheatcode();
}


// la mine explose 
void mineExplosee()
{
  if (animationUneFois)
  {
    // animation d'explosion
    animationUneFois = false;

    // vider l'ecran oled
    display.clearDisplay();
    display.display();

    Serial.println("BOOM !!!");

    previousMillisAlarme = millis();
    currentMillisAlarme = previousMillisAlarme;
    
    // activer l'alarme
    digitalWrite(ALARME_PIN, HIGH);
      
    // faire clignoter la led et l'ecran oled
    while(currentMillisAlarme - previousMillisAlarme < intervalAlarme)
    {
      currentMillisAlarme = millis();
      
      afficheText("BOOM", 3, true, true);
      clignoteLed(0, CRGB::Red, 1);
      afficheText("", 2, true, true);
    }
    
    // desactiver l'alarme
    digitalWrite(ALARME_PIN, LOW);
  }
  
  // la mine a deja explosee
  // heartbeat
  currentMillisHB = millis();
  if(currentMillisHB - previousMillisHB > intervalHB)
  {
    previousMillisHB = currentMillisHB;
    afficheText("CETTE MINE A DEJA EXPLOSE", 1, true, true);
    clignoteLed(0, CRGB::Blue, 1);
  }

  // rearmer la mine si le jumper REARM_PIN est présent
  if(digitalRead(REARM_PIN)==LOW)
  {
    animationUneFois = true;
	  statutMine = MINE_LASER_CALIBRAGE;
    Serial.println("LA MINE EST REARMEE");
    afficheText("LA MINE EST REARMEE", 1, true, true);
    delay(2000);
  }
}


// lire le photoresistor cX fois
int lirePhotoresistor(int cX)
{
  int resultat = 0;
  for (int i=0;i<cX;i++)
  {
    resultat+=analogRead(PHOTORESISTOR_PIN);
    delay(5);
  }

  resultat/=cX;

  return(resultat); 
}


// gestion d'un bouton poussoir
bool appuiSurBouton(int b)
{
  unsigned long int previousMillisPush = millis();
    
  if ( (digitalRead(b)==LOW) && (!appuiLong) )
  {
    delay(50);
    while( (digitalRead(b)==LOW) && (!appuiLong) )
    {
      if ( (millis()-previousMillisPush) > 3000 )
      {
        appuiLong = true;
        Serial.println("APPUI LONG");
      }
      // debounce delay
      delay(50);
    }
    return true;
  }
  else
  {
    if ( (digitalRead(b)==HIGH) && appuiLong )
    {
      appuiLong = false;
      Serial.println("APPUI LONG TERMINE");
    }    
    return false;
  }  
}



// faire clignoter cX fois la led cLed avec la couleur cCouleur
void clignoteLed(int cLed, CRGB cCouleur, int cX)
{
  for (int i=0;i<cX;i++)
  {
    leds[cLed] = cCouleur;
    FastLED.show();
    delay(50);
    leds[cLed] = CRGB::Black;
    FastLED.show();
    delay(50);
  } 
}


// afficher un texte sur l'ecran OLED
void afficheText(char texteAAfficher[], int taillePolice, bool videEcran, bool changementEcran)
{
  if (videEcran)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
  }

  display.setTextSize(taillePolice);
  display.println(texteAAfficher);

  if(changementEcran)
  {
    display.display();
  }
}


// gestion du cheatcode
void manageCheatcode()
{
  bool changement = false;
  
  // check si appui sur MODE_PIN
  if (digitalRead(MODE_PIN) != statutInterrupteurPrecedent)
  {
    statutInterrupteurPrecedent = !statutInterrupteurPrecedent;

    for (int i=1;i<TAILLE_CHEATCODE;i++)
    {
      cheatCodeActuel[i-1] = cheatCodeActuel[i];
    }
    cheatCodeActuel[TAILLE_CHEATCODE-1] = 1;
    changement = true;

    // sortie serial.print
    Serial.println("sw");
    for (int i=0;i<TAILLE_CHEATCODE;i++)
    {
      Serial.print(cheatCodeActuel[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }

  // check si appui sur BOUTON_ACTIVATION_PIN
  if (appuiSurBouton(BOUTON_ACTIVATION_PIN))
  {
    for (int i=1;i<TAILLE_CHEATCODE;i++)
    {
      cheatCodeActuel[i-1] = cheatCodeActuel[i];
    }
    cheatCodeActuel[TAILLE_CHEATCODE-1] = 0;
    changement = true;

    // sortie serial.print
    Serial.println("but");
    for (int i=0;i<TAILLE_CHEATCODE;i++)
    {
      Serial.print(cheatCodeActuel[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }

  if (changement)
  {
    // verifier si le cheatCodeActuel est le bon
    bool flagOK = true;
    for (int i=0;i<TAILLE_CHEATCODE;i++)
    {
      if (cheatCodeActuel[i] != sequenceCheatCode[i])
      {
        flagOK = false;
      }
    }
  
    if (flagOK)
    {
      // cheatCodeActuel est correct
      // reinitialiser cheatCodeActuel
      for (int i=0;i<TAILLE_CHEATCODE;i++)
      {
        cheatCodeActuel[i] = 9;
      }

      // reset variables
      animationUneFois = true;
      statutMine = MINE_ALLUMEE;

      // serial print & oled
      Serial.println("MINE DESACTIVATEE");

      afficheText("MINE", 2, true, false);
      afficheText("DESACTIVATEE", 1, false, true);
      delay(2000);
    }
  }  
}
/*
 * ----------------------------------------------------------------------------
 * FIN DU FONCTIONS ADDITIONNELLES
 * ----------------------------------------------------------------------------
 */
