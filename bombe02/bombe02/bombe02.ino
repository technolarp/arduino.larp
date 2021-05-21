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
 * un afficheur 7segment*4 avec un controleur TM1637
 * une LED rouge 5v et une résistance appropriée (220 ohms)
 * un buzzer piezo 5v
 * trois boutons poussoir NO (Normally Open)
 * 4 morceaux de fils électriques, avec en option des connecteurs de votre choix
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
 * A2     FILS_PIN[0]             fil de desamorcage n°1
 * A3     FILS_PIN[1]             fil de desamorcage n°2
 * D12    FILS_PIN[2]             fil de desamorcage n°3
 * D13    FILS_PIN[3]             fil de desamorcage n°4
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
enum {BOMBE_ALLUMEE, BOMBE_ACTIVE, BOMBE_EXPLOSEE, BOMBE_SAFE};
byte statutBombe = BOMBE_ALLUMEE;

// FILS DE DESAMORCAGE
#define FILS_NB 4
byte pinFils[FILS_NB] = {A2, A3, 12, 13};
enum {FIL_EXPLOSION, FIL_SAFE, FIL_DELAI, FIL_NEUTRE, FIL_ALEATOIRE, FIL_COUPE};

// DEFINITION DES ACTIONS POUR CHAQUE FIL
byte actionFil[FILS_NB] = {
  FIL_ALEATOIRE,       // fil 1 connecte sur la pin A2
  FIL_ALEATOIRE,       // fil 2 connecte sur la pin A3
  FIL_ALEATOIRE,       // fil 3 connecte sur la pin 12
  FIL_ALEATOIRE        // fil 4 connecte sur la pin 13
  };

// repartition des fils aleatoires
byte nbFilExplosion = 1;
byte nbFilSafe = 1;
byte nbFilDelai = 2;

byte filAssignation[FILS_NB] = {FIL_ALEATOIRE, FIL_ALEATOIRE, FIL_ALEATOIRE, FIL_ALEATOIRE};

// COMPTE A REBOURS
int minuteRestantes;
int secondeRestantes;

// HEARTBEAT
unsigned long int previousMillisHB;
unsigned long int currentMillisHB;
unsigned long int intervalHB;

// DOUBLE POINT
unsigned long int ul_PreviousMillisDoublePoint;
unsigned long int ul_CurrentMillisDoublePoint;
unsigned long int ul_IntervalDoublePoint;

bool doublePoints = true;

// DIVERS
#define MINUTE_PAR_DEFAUT 5
#define SECONDE_PAR_DEFAUT 0

bool animationUneFois = true;

enum {ANIM_VIDE, ANIM_EXPLOSEE, ANIM_SAFE, ANIM_BOOM};


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

  // configurer les 4 fils de desamorcage
  for (int i=0;i<FILS_NB;i++)
  {
    pinMode(pinFils[i], INPUT);
    digitalWrite(pinFils[i], HIGH);
  }

  // initialiser l'aleat
  randomSeed(analogRead(0));

  // temps restant par defaut
  minuteRestantes = MINUTE_PAR_DEFAUT;
  secondeRestantes = SECONDE_PAR_DEFAUT;

  animationUneFois = true;

  // beep une fois
  beepCourt();

  // init du temps heartbeat
  previousMillisHB = millis();
  currentMillisHB = previousMillisHB;
  intervalHB = 200;

  // init du temps du double point
  ul_PreviousMillisDoublePoint = millis();
  ul_CurrentMillisDoublePoint = previousMillisHB;
  ul_IntervalDoublePoint = 500;
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

    case BOMBE_SAFE:
      // animation de desamorcage
      bombeSafe();
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

    // gerer les fils aleatoires
    affecteFilsAleatoires();
  
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
      
      //display.showNumberDecEx(tempsAAfficher, 0b01000000, true, 4, 0);
    }

    // check si un fil a ete coupe
    checkFilCoupe();

    // changer le : on / off
    ul_CurrentMillisDoublePoint = millis();
    if(ul_CurrentMillisDoublePoint - ul_PreviousMillisDoublePoint > ul_IntervalDoublePoint)
    {
      ul_PreviousMillisDoublePoint = ul_CurrentMillisDoublePoint;
      doublePoints = !doublePoints;
  
      //displayDirty=true;
    }

    updateDisplay();
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
    for (int i=0;i<15;i++)
    {
      animation7Seg(ANIM_VIDE);
      delay(200);
      animation7Seg(ANIM_BOOM);
      delay(200);
    }
  }
  else
  {
    // la bombe a deja explosee
    // afficher "- - - -" sur le 4*7segment
    animation7Seg(ANIM_EXPLOSEE);
    delay(500);
  }
}

// la bombe est desamorcee
void bombeSafe()
{
// animation de desamorcage
  if (animationUneFois)
  {
    animationUneFois = false;
    
    // beep court
    beepCourt();

    // eteindre la led rouge
    digitalWrite(LED_ROUGE_PIN, LOW);

    // afficher alternativement "S A F E" et "      "    
    for (int i=0;i<5;i++)
    {
      animation7Seg(ANIM_VIDE);
      delay(200);
      animation7Seg(ANIM_SAFE);
      delay(200);
    }
  }
  else
  {
    // la bombe est deja desamorcee
    // afficher alternativement "S A F E" et le temps restant
    int tempsAAfficher = minuteRestantes*100 + secondeRestantes;
    display.showNumberDecEx(tempsAAfficher, 0b01000000, true, 4, 0);
    delay(1000);
    animation7Seg(ANIM_SAFE);
    delay(1000);

    
  }  
}

void animation7Seg(byte toShow)
{
  //definir les segments du display
  uint8_t SEG_VIDE[] = {0x00, 0x00, 0x00, 0x00};
  uint8_t SEG_BOOM[] = {0x7C, 0x5C, 0x5C, 0x54};          // b, o, o, n
  uint8_t SEG_EXPLOSEE[] = {SEG_G, SEG_G, SEG_G, SEG_G};  // -, -, -, -
  uint8_t SEG_SAFE[] = {0x6D, 0x77, 0x71, 0x79};          //S, A, F, E

  switch (toShow) 
  {
    case ANIM_VIDE:
      display.setSegments(SEG_VIDE, 4, 0);
    break;

    case ANIM_EXPLOSEE:
      display.setSegments(SEG_EXPLOSEE, 4, 0);
    break;
    
    case ANIM_SAFE:
      display.setSegments(SEG_SAFE, 4, 0);
    break;

    case ANIM_BOOM:
      display.setSegments(SEG_BOOM, 4, 0);
    break;
    
    default:
      // do nothing      
    break;
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


// check fil coupe
void checkFilCoupe()
{
  // scan all wires
  for (int i=0;i<FILS_NB;i++)
  {
    // if the i wire was not previously cutted ans is now cut
  if ( (actionFil[i] != FIL_COUPE) && (digitalRead(pinFils[i]) == HIGH) )
    { 
      switch (actionFil[i]) 
      {
        case FIL_NEUTRE:
          // wire is neutral, nothing to do
          actionFil[i]=FIL_COUPE;
        break;
        
        case FIL_DELAI:
          // divide ul_Interval by 2
          doubleBeep();
          intervalHB/=2;
          actionFil[i]=FIL_COUPE;
        break;
    
        case FIL_SAFE:
          // if bomb status already changed to BOMBE_EXPLOSEE, don't change to BOMB_SAFE
          if (statutBombe != BOMBE_EXPLOSEE)
          {
            // bomb is now safe
            beepCourt();
            statutBombe = BOMBE_SAFE;
          }
          actionFil[i]=FIL_COUPE;
        break;
        
        case FIL_EXPLOSION:
          // detonate the bomb
          statutBombe = BOMBE_EXPLOSEE;
          actionFil[i]=FIL_COUPE;
        break;
        
        default:
          // do nothing      
        break;
      }
    }    
  }
}



void affecteFilsAleatoires()
{
  int filsAleatoiresCpt = 0;
  int filsAleatoiresCptCopie = 0;
  int indexfilAssignation = 0;
  for (int i=0;i<FILS_NB;i++)
  {
    if (actionFil[i]==FIL_ALEATOIRE)
    {
      filsAleatoiresCpt++;
    }
  }

  filsAleatoiresCptCopie=filsAleatoiresCpt;
  
  // prepare FIL_EXPLOSION wires
  while( (filsAleatoiresCpt>0) && (nbFilExplosion>0) )
  {
    filAssignation[indexfilAssignation]=FIL_EXPLOSION;
    indexfilAssignation++;
    nbFilExplosion--;
    filsAleatoiresCpt--;
  }

  // prepare FIL_SAFE wires
  while( (filsAleatoiresCpt>0) && (nbFilSafe>0) )
  {
    filAssignation[indexfilAssignation]=FIL_SAFE;
    indexfilAssignation++;
    nbFilSafe--;
    filsAleatoiresCpt--;
  }

  // prepare FIL_DELAI wires
  while( (filsAleatoiresCpt>0) && (nbFilDelai>0) )
  {
    filAssignation[indexfilAssignation]=FIL_DELAI;
    indexfilAssignation++;
    nbFilDelai--;
    filsAleatoiresCpt--;
  }

  // create an array of random number between 10 and 50
  int tabAleatoire[FILS_NB];
  for (int i=0;i<FILS_NB;i++)
  {
    tabAleatoire[i]=random(10,50);
  }
    
  // assign new wire in actionFil[]  
  for (int i=0;i<FILS_NB;i++)
  {
    if (actionFil[i]==FIL_ALEATOIRE)
    {
      // find the greater index in tabAleatoire
      int indexToUse = indexMaxValeur(filsAleatoiresCptCopie, tabAleatoire);
      tabAleatoire[indexToUse]=0;
      
      // assign the new wire type with the random index
      actionFil[i]=filAssignation[indexToUse];
    }
  }
}

// index of max value in an array
int indexMaxValeur(int arraySize, int arrayToSearch[])
{
  int indexMax=0;
  int currentMax=0;
  
  for (int i=0;i<arraySize;i++)
  {
    if (arrayToSearch[i]>=currentMax)
    {
      currentMax = arrayToSearch[i];
      indexMax = i;
    }
  }
  
  return(indexMax);
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

// display function
void updateDisplay()
{
    int displayTime = minuteRestantes*100 + secondeRestantes;
    
    // update 7seg display to display new time
    if (doublePoints)
    {
      //  with clockpoints
      display.showNumberDecEx(displayTime, 0b01000000, true, 4, 0);
    }
    else
    {
      //  without clockpoints
      display.showNumberDecEx(displayTime, 0b00000000, true, 4, 0);
    }
}
/*
 * ----------------------------------------------------------------------------
 * FIN DU FONCTIONS ADDITIONNELLES
 * ----------------------------------------------------------------------------
 */
