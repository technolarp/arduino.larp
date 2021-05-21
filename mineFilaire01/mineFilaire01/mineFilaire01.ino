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
 * Ce montage permet de simuler une bombe/mine filaire pour un jeu de role 
 * grandeur nature ou une partie d'airsoft
 * !! CE N'EST PAS UNE VERITABLE BOMBE !! 
 * 
 * Vous aurez besoin
 * une carte Arduino uno ou nano
 * des fils et un jumper (cavalier) type carte mere de PC
 * une alarme 5-12v et 1 mosfet 2N7000
 * une LED neopixel ou ws2811
 * une bouton poussoir NO (Normally Open)
 * 
 * Vous aurez besoin d'installer la bibliothèque FastLED.h dans l'IDE
 * 
 * ----------------------------------------------------------------------------
 */
 
/*
 * ----------------------------------------------------------------------------
 * PINOUT 
 * D2     LED_DATA_PIN             led RGB neoPixel ou ws2811
 * D3     BOUTON_ACTIVATION_PIN   bouton d'activation
 * D5     FIL_PIEGE_PIN           fil piege
 * D7     ALARME_PIN              mosfet + alarme
 * ----------------------------------------------------------------------------
 */


// LED RGB
#include <FastLED.h>  // https://github.com/FastLED/FastLED

#define NB_LEDS 1
#define LED_DATA_PIN 2

// tableau de leds neopixel
CRGB leds[NB_LEDS];



// COMMANDES & FIL PIEGE
#define FIL_PIEGE_PIN 5
#define BOUTON_ACTIVATION_PIN 3
#define ALARME_PIN 7

// STATUT DE LA MINE
enum {MINE_ALLUMEE, MINE_FIL_PIEGE_ACTIVE, MINE_EXPLOSEE};
byte statutMine = MINE_ALLUMEE;

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

  // COMMANDES & FIL PIEGE
  pinMode(FIL_PIEGE_PIN, INPUT);
  digitalWrite(FIL_PIEGE_PIN, HIGH);
  
  pinMode(BOUTON_ACTIVATION_PIN, INPUT);
  digitalWrite(BOUTON_ACTIVATION_PIN, HIGH);

  pinMode(ALARME_PIN, OUTPUT);
  digitalWrite(ALARME_PIN, LOW);

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
    clignoteLed(0, CRGB::Green, 1);
  }

  // detection de l'activation par le bouton poussoir
  if (appuiSurBouton(BOUTON_ACTIVATION_PIN))
  {
    statutMine = MINE_FIL_PIEGE_ACTIVE;
    Serial.println("ACTIVATION DU MODE FIL PIEGE");
  }
}



// la mine est activee
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
    clignoteLed(0, CRGB::Red, 1);
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
}

  

// gestion d'un bouton poussoir
bool appuiSurBouton(int b)
{
  if (digitalRead(b)==LOW)
  {
    delay(100);
    while(digitalRead(b)==LOW)
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
