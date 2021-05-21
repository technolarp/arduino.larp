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
 * Ce montage permet de lire des tag RFID et de copier l'UID sur un port COM du PC
 * 
 * Vous aurez besoin
 * une carte Arduino uno ou nano
 * un lecteur RFID RC522
 * 
 * Vous aurez besoin d'installer la bibliothèque MFRC522.h dans l'IDE
 * https://github.com/miguelbalboa/rfid
 * 
 * Ce sketch est basé sur https://github.com/miguelbalboa/rfid/tree/master/examples/DumpInfo
 * 
 * ----------------------------------------------------------------------------
 */

 
/*
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance



void setup() 
{
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	
  // LED
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i=0;i<5;i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void loop() 
{
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) 
	{
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) 
	{
		return;
	}

  // led ON
  digitalWrite(LED_BUILTIN, HIGH);
      
  //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    if (mfrc522.uid.uidByte[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println("");

  // led OFF
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  mfrc522.PICC_HaltA(); // Halt PICC    
}



/*
 * tag NTAG213
 * https://arduino.stackexchange.com/questions/55227/arduino-uno-how-to-write-personal-data-in-ntag213-using-rfid-rc522
 * 
 */
