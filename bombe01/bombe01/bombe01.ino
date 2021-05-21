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
 * Ce montage permet de simuler une bombe pour un jeu de role 
 * grandeur nature ou une partie d'airsoft
 * !! CE N'EST PAS UNE VERITABLE BOMBE !! 
 * 
 * Vous aurez besoin
 * une carte Arduino uno ou nano
 * un afficheur 7segment*4 avec un controleur TM1637
 * une LED rouge 5v et une résistance appropriée (220 ohms)
 * un buzzer piezo 5v
 * trois boutons poussoir NO (Normally Open)
 *
 * Vous aurez besoin d'installer la bibliothèque TM1637Display.h dans l'IDE
 * 
 * ----------------------------------------------------------------------------
 */

/*
 * ----------------------------------------------------------------------------
 * PINOUT
 * D5     LED_ROUGE_PIN           led rouge + resistance
 * D6     ALARME_PIN              buzzer piezo 5v
 * D7     CLK_PIN                 TM1637 pin CLK
 * D8     DIO_PIN                 TM1637 pin DIO
 * D9     BOUTON_BAS_PIN          bouton haut
 * D10    BOUTON_HAUT_PIN         bouton bas
 * D11    BOUTON_ACTIVATION_PIN   bouton d'activation
 * 
 * ----------------------------------------------------------------------------
 */ 

#include <Arduino.h>

// AFFICHEUR 7SEGMENTS TM1637
#include <TM1637Display.h>    // https://github.com/avishorp/TM1637

#define CLK_PIN 7
#define DIO_PIN 8

TM1637Display display(CLK_PIN, DIO_PIN);


// LED ROUGE
#define LED_ROUGE_PIN 5
bool statutLedRouge;

// COMMANDES
#define ALARME_PIN 6
#define BOUTON_BAS_PIN 9
#define BOUTON_HAUT_PIN 10
#define BOUTON_ACTIVATION_PIN 11

// STATUT DE LA BOMBE
enum {BOMBE_ALLUMEE, BOMBE_ACTIVE, BOMBE_EXPLOSEE};
byte statutBombe = BOMBE_ALLUMEE;


// COMPTE A REBOURS
int minuteRestantes;
int secondeRestantes;

// HEARTBEAT
unsigned long int previousMillisHB;
unsigned long int currentMillisHB;
unsigned long int intervalHB;


// DIVERS
#define MINUTE_PAR_DEFAUT 5
#define SECONDE_PAR_DEFAUT 0

bool animationUneFois = true;



/*
 * ----------------------------------------------------------------------------
 * SETUP
 * ----------------------------------------------------------------------------
 */
void setup() 
{
  // configurer l'afficheur 7segment
  display.setBrightness(0x0f);
 
  // configurer le bouton d'activation
  pinMode(BOUTON_ACTIVATION_PIN, INPUT);
  digitalWrite(BOUTON_ACTIVATION_PIN, HIGH);

  // configurer le bouton haut
  pinMode(BOUTON_HAUT_PIN, INPUT);
  digitalWrite(BOUTON_HAUT_PIN, HIGH);

  // configurer le bouton bas
  pinMode(BOUTON_BAS_PIN, INPUT);
  digitalWrite(BOUTON_BAS_PIN, HIGH);

  // configurer la led rouge
  statutLedRouge = LOW;
  pinMode(LED_ROUGE_PIN, OUTPUT);
  digitalWrite(LED_ROUGE_PIN, statutLedRouge);

  // temps restant par defaut
  minuteRestantes = MINUTE_PAR_DEFAUT;
  secondeRestantes = SECONDE_PAR_DEFAUT;

  animationUneFois = true;

  // beep une fois
  beepCourt();

  previousMillisHB = millis();
  currentMillisHB = previousMillisHB;
  intervalHB = 200;
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
  // loop en fonction du statut de la bombe
  switch (statutBombe) 
  {
    case BOMBE_ALLUMEE:
      // configurer le temps restant
      bombeAllumee();
    break;

    case BOMBE_ACTIVE:
      // la bombe est active
      bombeActive();
    break;
    
    case BOMBE_EXPLOSEE:
      // animation d'explosion
      bombeExplosee();
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

// attendre l'activation et le temps restant
void bombeAllumee()
{
  // mettre a jour l'afficheur
  int tempsAAfficher = minuteRestantes*100 + secondeRestantes;
  display.showNumberDecEx(tempsAAfficher, 0b01000000, true, 4, 0);

  // faire clignoter la led rouge
  currentMillisHB = millis();
  if(currentMillisHB - previousMillisHB > intervalHB)
  {
    previousMillisHB = currentMillisHB;

    // allumer ou eteinfdre la led rouge
    statutLedRouge = !statutLedRouge;
    digitalWrite(LED_ROUGE_PIN, statutLedRouge);
  }

  // check si le temps a changer
  // BOUTON_HAUT_PIN appuyé, on augmente le temps
  if (appuiSurBouton(BOUTON_HAUT_PIN))
  {
    minuteRestantes++;
	// on ne depasse pas 60 minutes
    minuteRestantes=min(60, minuteRestantes);
  }

  // BOUTON_BAS_PIN  appuyé, on diminue le temps
  if (appuiSurBouton(BOUTON_BAS_PIN))
  {
    minuteRestantes--;
	// on ne depasse pas 0 minutes
    minuteRestantes=max(0, minuteRestantes);
  }

  // check si la bombe est activée
  // BOUTON_ACTIVATION_PIN appuyé, on active la bombe
  if (appuiSurBouton(BOUTON_ACTIVATION_PIN))
  {
    // la bombe est maintenant active
	statutBombe = BOMBE_ACTIVE;

    // beep 2 fois
    doubleBeep();
  
    // allumer la led rouge
    digitalWrite(LED_ROUGE_PIN, HIGH);
  
    // changer le intervalHB pour 1000 ms
    intervalHB = 1000;
    previousMillisHB = millis();
  }
}

// la bombe est active
void bombeActive()
{  
  // check du temps restant
  if ( (secondeRestantes == 0) && (minuteRestantes == 0) )
  {
    // le compte a rebours est terminé !!
    statutBombe = BOMBE_EXPLOSEE;
  }
  else
  {
    // il reste du temps
    // attendre 1 seconde
    currentMillisHB = millis();
    if(currentMillisHB - previousMillisHB > intervalHB)
    {
      previousMillisHB = currentMillisHB;
  
      // diminuer le temps d'une seconde
      secondeRestantes--;
      
      // gérer le changement de minute
      if (secondeRestantes == -1)
      {
        secondeRestantes = 59;
        minuteRestantes--;
      }
  
      // beep toutes les 30 seconde
      if ( (secondeRestantes == 0) || (secondeRestantes == 30) )
      {
        beepCourt();
      }
  
      // beep toutes les secondes quand il reste 10 secondes ou moins
      if ( (minuteRestantes == 0) && (secondeRestantes <= 10) )
      {
        beepCourt();
      }
    
      // mettre a jour l'afficheur
      int tempsAAfficher = minuteRestantes*100 + secondeRestantes;
    
      // faire clignoter le : sur l'afficheur
      display.showNumberDecEx(tempsAAfficher, 0b01000000, true, 4, 0);
    }
  }
}


// la bombe explose
void bombeExplosee()
{
  // animation d'explosion
  if (animationUneFois)
  {
    animationUneFois = false;
    
    // beep long
    beepLong();

    // eteindre la led rouge
    digitalWrite(LED_ROUGE_PIN, LOW);

    // afficher alternativement "b o o n" et "      "
	uint8_t SEG_VIDE[] = {0x00, 0x00, 0x00, 0x00};
    uint8_t SEG_BOOM[] = {0x7C, 0x5C, 0x5C, 0x54}; // b, o, o, n
    
    for (int i=0;i<15;i++)
    {
      display.setSegments(SEG_VIDE, 4, 0);
      delay(200);
      display.setSegments(SEG_BOOM, 4, 0);
      delay(200);
    }
  }
  else
  {
    // la bombe a deja explosee
    // afficher "- - - -" sur le 4*7segment
    uint8_t SEG_EXPLOSEE[] = {SEG_G, SEG_G, SEG_G, SEG_G};
    display.setSegments(SEG_EXPLOSEE);
  }
}





// gestion d'un bouton poussoir
bool appuiSurBouton(int b)
{
  if (digitalRead(b)==LOW)
  {
    delay(50);
    while(digitalRead(b)==LOW)
    {
      // delai d'attente de debounce
      delay(50);
    }
    return true;
  }
  else
  {
    return false;
  }
}




// fonctions pour faire beeper le buzzer
void beepCourt() 
{
  beep(1000, 50);
}

void doubleBeep()
{
  beep(1000, 50);
  delay(200);
  beep(1000, 50);
}
 
void beepLong() 
{
  beep(1000, 5000);
}


void beep(int freqBeep, int delayBeep)
{
  tone(ALARME_PIN, freqBeep, delayBeep);
}

/*
 * ----------------------------------------------------------------------------
 * FIN DU FONCTIONS ADDITIONNELLES
 * ----------------------------------------------------------------------------
 */
