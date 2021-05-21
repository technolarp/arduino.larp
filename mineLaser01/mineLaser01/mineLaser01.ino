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
 * une alarme 5-12v et un mosfet 2N7000
 * une LED neopixel ou ws2811
 * une bouton poussoir NO (Normally Open)
 * un jumper de carte mere de PC ou un interrupteur on/off
 * 
 * Vous aurez besoin d'installer la bibliothèque FastLED.h dans l'IDE
 * 
 * ----------------------------------------------------------------------------
 */
 
/*
 * ----------------------------------------------------------------------------
 * PINOUT
 * A0     PHOTORESISTOR_PIN       photoresistor
 * D2     LED_DATA_PIN            led RGB neoPixel ou ws2811
 * D3     BOUTON_ACTIVATION_PIN   bouton d'activation
 * D6     REARM_PIN 6             jumper/interrupteur pour rearmement
 * D7     ALARME_PIN              mosfet + alarme
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
#define ALARME_PIN 7

int laserValeurReference;
int laserMarge = 15;
int laserValeurActuelle;

// STATUT DE LA MINE
enum {MINE_ALLUMEE, MINE_LASER_CALIBRAGE, MINE_LASER_ACTIVE, MINE_EXPLOSEE};
byte statutMine = MINE_ALLUMEE;

// REARMEMENT
#define REARM_PIN 6

// HEARTBEAT
unsigned long int previousMillisHB;
unsigned long int currentMillisHB;
unsigned long int intervalHB;

// DIVERS
bool animationUneFois = true;



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
  FastLED.addLeds<WS2811, LED_DATA_PIN>(leds, NB_LEDS);
  // pour utiliser une led adafruit neopixel
  //FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NB_LEDS);
  FastLED.setBrightness(50);

  // COMMANDES & LASER
  pinMode(PHOTORESISTOR_PIN, INPUT);
  
  pinMode(BOUTON_ACTIVATION_PIN, INPUT);
  digitalWrite(BOUTON_ACTIVATION_PIN, HIGH);

  pinMode(ALARME_PIN, OUTPUT);
  digitalWrite(ALARME_PIN, LOW);

  // REARMEMENT
  pinMode(REARM_PIN, INPUT);
  digitalWrite(REARM_PIN, HIGH);

  // HEARTBEAT
  previousMillisHB = millis();
  currentMillisHB = previousMillisHB;
  intervalHB = 1000;


  // DIVERS
  animationUneFois = true;

  Serial.println("START !!!");
  Serial.println("EN ATTENTE D'ACTIVATION");
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
    clignoteLed(0, CRGB::Green, 1);
  }

  // detection de l'activation par le bouton poussoir
  if (appuiSurBouton(BOUTON_ACTIVATION_PIN))
  {
    statutMine = MINE_LASER_CALIBRAGE;
    Serial.println("CALIBRAGE DU PHOTORESITOR");
  }
}


// la mine est activee
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
    clignoteLed(0, CRGB::Red, 1);

    Serial.println(laserValeurActuelle);
  }
}


// la mine explose 
void mineExplosee()
{
  if (animationUneFois)
  {
    // animation d'explosion
    animationUneFois = false;

    Serial.println("BOOM !!!");
    
    // activer l'alarme
    digitalWrite(ALARME_PIN, HIGH);
      
    // faire clignoter la led
    for (int i=0;i<10;i++)
    {
      clignoteLed(0, CRGB::Red, 2);      
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
    clignoteLed(0, CRGB::Blue, 1);
  }

  // rearmer la mine si le jumper  REARM_PIN n'est pas présent
  if(digitalRead(REARM_PIN))
  {
    animationUneFois = true;
	statutMine = MINE_LASER_CALIBRAGE;
    Serial.println("LA MINE EST REARMEE");
  }
}


// calculer la luminosité sur le photoresistor
void laser_calibrage()
{
  laserValeurReference = 0;
  laserValeurActuelle = 0;

  Serial.println("CALIBRAGE");
  
  for (int i=0;i<20;i++)
  {
    laserValeurActuelle+=lirePhotoresistor(5);
    
    // animation de calibrage
    clignoteLed(0, CRGB::Orange, 1);
    
  }

  laserValeurActuelle/=20;
  laserValeurReference = laserValeurActuelle;
  Serial.print("laserValeurReference: ");
  Serial.println(laserValeurReference);

  // activer la mine laser
  statutMine = MINE_LASER_ACTIVE;
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
  if (digitalRead(b)==false)
  {
    delay(100);
    while(digitalRead(b)==false)
    {
      // debounce delay
      delay(100);
    }
    return true;
  }
  else
  {
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
    delay(100);
    leds[cLed] = CRGB::Black;
    FastLED.show();
    delay(100);
  } 
}

/*
 * ----------------------------------------------------------------------------
 * FIN DU FONCTIONS ADDITIONNELLES
 * ----------------------------------------------------------------------------
 */
