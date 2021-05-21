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
 * Ce montage permet de simuler e simuler une serrure à clavier matriciel 
 * pour le jeu de rôle Grandeur Nature (jdr GN) & l'airsoft, à base d'arduino
 * 
 * Vous aurez besoin
 * une carte Arduino uno ou nano
 * un clavier matriciel 4*4
 * un buzzer piezo 5V
 * une LED rouge 5v et une LED verte 5v, avec leur résistance
 *
 * Vous aurez besoin d'installer la bibliothèque keypad.h dans l'IDE
 * 
 * ----------------------------------------------------------------------------
 */

/*
 * ----------------------------------------------------------------------------
 * PINOUT
 * D2     KEYPAD PIN 8          clavier matriciel
 * D3     KEYPAD PIN 7          clavier matriciel
 * D4     KEYPAD PIN 6          clavier matriciel
 * D5     KEYPAD PIN 5          clavier matriciel
 * D6     KEYPAD PIN 4          clavier matriciel
 * D7     KEYPAD PIN 3          clavier matriciel
 * D8     KEYPAD PIN 2          clavier matriciel
 * D9     KEYPAD PIN 1          clavier matriciel
 * D10    LED_ROUGE_PIN         led rouge + resistance 220 Ohms
 * D11    LED_VERTE_PIN         led verte + resistance 220 Ohms
 * D12    BUZZER_PIN            buzzer piezo 5v
 * 
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>


// CODE DE LA SERRURE
char codeSerrure[4]={'1','2','3','4'};

// BLOCAGE DE LA SERRURE
int erreurCodeMax = 3;
int tempoBlocage = 30;

// LED
#define LED_ROUGE_PIN 10
#define LED_VERTE_PIN 11

// BUZZER
#define BUZZER_PIN 12

// STATUTS DE LA SERRURE
enum {
  SERRURE_OUVERTE=0, 
  SERRURE_FERMEE=1,
  SERRURE_BLOQUEE=2
  };

byte statutSerrureActuel=SERRURE_FERMEE;
byte statutSerrurePrecedent=SERRURE_FERMEE;

// CLAVIER
#include <Keypad.h>   // https://playground.arduino.cc/Code/Keypad/

const byte NB_LIGNES = 4; //four NB_LIGNES
const byte NB_COLONNES = 4; //four columns
char touchesClavier[NB_LIGNES][NB_COLONNES] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte lignesPin[NB_LIGNES] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colonnesPin[NB_COLONNES] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad clavier = Keypad( makeKeymap(touchesClavier), lignesPin, colonnesPin, NB_LIGNES, NB_COLONNES );


// DIVERS
bool clignoteLed = false;
int delaiLed = 200;
bool codeOK = false;

int nbFauxCode=0;
char codeSerrureActuel[4]={'0','0','0','0'};


// REFERENCE TEMPS BLOCAGE
unsigned long int previousMillisBlocage;
unsigned long int currentMillisBlocage;
unsigned long int intervalBlocage;



/*
 * ----------------------------------------------------------------------------
 * SETUP
 * ----------------------------------------------------------------------------
 */
void setup()
{
  // on initialise la sortie serie pour debug
  Serial.begin(115200);

  // initialisation des led rouge & verte
  pinMode(LED_VERTE_PIN, OUTPUT);
  pinMode(LED_ROUGE_PIN, OUTPUT);

  changerEtatLed(LED_VERTE_PIN, LOW);
  changerEtatLed(LED_ROUGE_PIN, HIGH);

  // initialisation du buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // calcul du delai de blocage
  intervalBlocage=tempoBlocage*1000;
  
  // beep one time
  beepCourt();

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
  // gerer le statut de la serrure
  switch (statutSerrureActuel)
    {
    case SERRURE_FERMEE:
    // la serrure est fermee
    serrureFermee();
    break;
  
    case SERRURE_OUVERTE:
    // la serrure est ouverte
    serrureOuverte();
    break;
  
    case SERRURE_BLOQUEE:
    // la serrure est bloquee
    serrureBloquee();
    break;
    
    default:
    // nothing
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
void serrureFermee()
{
  // on allume la led rouge, on eteint la led verte
  changerEtatLed(LED_VERTE_PIN, LOW);
  changerEtatLed(LED_ROUGE_PIN, HIGH);

  // on check si une touche du clavier a ete activee
  appuiClavier();
}

void serrureOuverte()
{
  // on eteint la led rouge, on allume la led verte
  changerEtatLed(LED_VERTE_PIN, HIGH);
  changerEtatLed(LED_ROUGE_PIN, LOW);

  // on check si une touche du clavier a ete activee
  appuiClavier();
}

    
void serrureBloquee()
{ 
  // la serrure est bloquer
  // on verifie si le delai de blocage est termine
  currentMillisBlocage = millis();
  if(currentMillisBlocage - previousMillisBlocage > intervalBlocage)
  {
    // le blocage est termine
    statutSerrureActuel=statutSerrurePrecedent;
    nbFauxCode=0;
  }
  else
  {
    // le blocage actif, on clignote les led
    clignoteLed=!clignoteLed;
    changerEtatLed(LED_VERTE_PIN, clignoteLed);
    changerEtatLed(LED_ROUGE_PIN, !clignoteLed);
    delay(delaiLed);
  }
}



// manage the keypad
void appuiClavier()
{
  char touche = clavier.getKey();  

  // une touche a ete pressee
  if (touche && statutSerrureActuel!=SERRURE_BLOQUEE)
  {
    beepCourt();
    
    // la touche pressee n'est pas un # et n'est pas un *
    if ((touche != '#') && (touche != '*'))
    {
      // on met a jour codeSerrureActuel[] en decalant chaque caractere
      codeSerrureActuel[0] = codeSerrureActuel[1];
      codeSerrureActuel[1] = codeSerrureActuel[2];
      codeSerrureActuel[2] = codeSerrureActuel[3];
      codeSerrureActuel[3] = touche;
    }
    else
    // la touche pressee est un # ou un *
    {
      // on compare codeSerrureActuel[] et codeSerrure[]
      codeOK=true;

      for (int i=0;i<4;i++)
      {
         if (codeSerrureActuel[i] != codeSerrure[i])
         {
          // ce caractere est faux, le code n'est pas bon
          codeOK=false;
         }
      }

      if (codeOK)
      {
        // le code est correct, on change le statut de la serrure
        delay(100);
        beepCourt();
        
        statutSerrureActuel=!statutSerrureActuel;
        nbFauxCode=0;
        
        codeSerrureActuel[0] = 0;
        codeSerrureActuel[1] = 0;
        codeSerrureActuel[2] = 0;
        codeSerrureActuel[3] = 0;
      }
      // le code est incorrect
      else
      {
        // on beep long
        beepLong();

        // on augmente le compteur de code faux
        nbFauxCode+=1;

        // on clignote la led rouge
        changerEtatLed(LED_VERTE_PIN, LOW);
        for (int i=0;i<4;i++)
        {
          changerEtatLed(LED_ROUGE_PIN, LOW);
          delay(delaiLed);        
          changerEtatLed(LED_ROUGE_PIN, HIGH);
          delay(delaiLed);
        }

        // si il y a eu trop de faux codes
        if (nbFauxCode>=erreurCodeMax)
        {
          // on bloque la serrure
          statutSerrurePrecedent=statutSerrureActuel;
          statutSerrureActuel=SERRURE_BLOQUEE;
          
          currentMillisBlocage = millis();
          previousMillisBlocage = currentMillisBlocage;
        }
      }
    }
  }

  // on attend un peu
  delay(5);
}


void faireClignoterLeds()
{
  for (int i=0;i<5;i++)
  {
    changerEtatLed(LED_VERTE_PIN, HIGH);
    changerEtatLed(LED_ROUGE_PIN, LOW);
    delay(delaiLed);
  
    changerEtatLed(LED_VERTE_PIN, LOW);
    changerEtatLed(LED_ROUGE_PIN, HIGH);
    delay(delaiLed);
  }
}

void changerEtatLed(int ledPin, int onoff)
{
  digitalWrite(ledPin, onoff);
}



// fonctions pour faire beeper le buzzer
void beepCourt() 
{
  beep(1000, 50);
}

 
void beepLong() 
{
  beep(1000, 1000);
}


void beep(int freqBeep, int delayBeep)
{
  tone(BUZZER_PIN, freqBeep, delayBeep);
}

/*
 * ----------------------------------------------------------------------------
 * FIN DU FONCTIONS ADDITIONNELLES
 * ----------------------------------------------------------------------------
 */
