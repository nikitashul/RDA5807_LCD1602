/*
    | Device name               | Device Pin / Description  |  Arduino Pin  |
    | ----------------          | --------------------      | ------------  |
    |     Display LCD1602       |                           |               |
    |                           | RST (RESET)               |      12       |
    |                           | EN  (Enable)              |      11       |
    |                           | D0                        |      5        |
    |                           | D1                        |      4        |
    |                           | D2                        |      3        |
    |                           | D3                        |      2        |
    |     RDA5807               |                           |               |
    |                           | SDA (pin 1)               |     A4        |
    |                           | SCK (pin 2)               |     A5        |
    |     Buttons               |                           |               |
    |                           | Volume Up                 |               |
    |                           | Volume Down               |               |
    |                           | Stereo/Mono               |               |
    |                           | RDS ON/off                |               |
    |                           | SEEK (encoder button)     |               |
    |     Encoder               |                           |               |
    |                           | A                         |               |
    |                           | B                         |               |
*/

#include <Wire.h>
#include <Arduino.h>
#include <RDA5807.h>
#include <LiquidCrystal.h>

#define DEBUG_INIT 1 //Debug on/off

#define RS 12
#define EN 11
#define D0 5
#define D1 4
#define D2 3
#define D3 2

RDA5807 rx; 

//unsigned int FM_STATION_FREQ = 10450; //104.50 Olimp
//unsigned int FM_STATION_FREQ = 10290; //102.90 Intervolna
uint16_t FM_STATION_FREQ = 10290; //102.90 Intervolna

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2, BACKLIGHT_PIN, POSITIVE );
LiquidCrystal lcd(RS, EN, D0, D1, D2, D3); //(rs, enable, d0, d1, d2, d3)

// Creat a set of new characters
byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};

byte armsUp[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b01010
};

byte frownie[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b10001
};

/*
    Show some basic information on display
*/
void showStatus_on_lcd()
{

  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("CHANELL:        ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRealChannel());
  delay(2000);
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("REAL_FREQ:      ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRealFrequency());
  delay(2000);
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("RSSI:           ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRssi());
  
}


void setup() {
  //--------------------------------------------------------
  //-------------------------USART--------------------------
  //--------------------------------------------------------
  Serial.begin(9600);
  if (DEBUG_INIT){
    Serial.println("Serial.begin(9600); ------ OK");
    delay(1000);
  }
  //--------------------------------------------------------
  
  //--------------------------------------------------------
  //--------------------------LCD---------------------------
  //--------------------------------------------------------
  lcd.begin(16,2);                  // initialize the lcd 
  lcd.createChar (0, smiley);       // load character to the LCD
  lcd.createChar (1, armsUp);       // load character to the LCD
  lcd.createChar (2, frownie);      // load character to the LCD
  lcd.home ();                      // go home
  lcd.print("Hello,   RDA5807");  
  lcd.setCursor ( 0, 1 );           // go to the next line
  lcd.print(" -----TEST----- ");      
  if (DEBUG_INIT){
    Serial.println("lcd.begin(); --- DEBUG --- OK");
    delay(1000);
  }
  //--------------------------------------------------------
 
  //--------------------------------------------------------
  //-------------------------RDA5807------------------------
  //--------------------------------------------------------
  rx.setup();
  rx.setVolume(6);
  rx.setBass(true);                 // Sets Bass Boost (value	FALSE = Disable; TRUE = Enable)
  rx.setMono(true);                 // Force mono (value	TRUE = Mono; FALSE force stereo)
  // rx.setRBDS(true);              // set RDS and RBDS. See setRDS.
  rx.setRDS(false);                 // set RDS.
  rx.setRBDS(false);                // set the RBDS operation.
  rx.setRdsFifo(false);
  rx.setFmDeemphasis(true);         //de	0 = 75 μs; 1 = 50 μs (75 μs. Used in USA (default); 50 μs. Used in Europe, Australia, Japan.)
  rx.setFrequency(FM_STATION_FREQ); // set STATION Freq.
  
  
  if (DEBUG_INIT){
    Serial.println("rx.setup(); RDA5807 ------ OK");
    delay(1000);
  }
  //--------------------------------------------------------
  
}

void loop() 
{
  
}