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
    |                           | SDIO (pin 8)              |     A4        |
    |                           | SCLK (pin 7)              |     A5        |
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

#include <Arduino.h>
#include <RDA5807.h>
//bool checkI2C();
#include <Wire.h>
#include <LiquidCrystal.h>

RDA5807 rx; 

unsigned int FM_STATION_FREQ = 10450; //104.50 Olimp
//unsigned int FM_STATION_FREQ = 10290; //102.90 Intervolna


#define   CONTRAST_PIN   9
#define   BACKLIGHT_PIN  7
#define   CONTRAST       110

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2, BACKLIGHT_PIN, POSITIVE );
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //(rs, enable, d0, d1, d2, d3)

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

/*********************************************************
   RDS
 *********************************************************/
char *rdsMsg;
char *stationName;
char *rdsTime;
char bufferStatioName[16];
char bufferRdsMsg[40];
char bufferRdsTime[20];
long stationNameElapsed = millis();
long polling_rds = millis();
long clear_fifo = millis();

void showRDSMsg()
{
  
  rdsMsg[22] = bufferRdsMsg[22] = '\0';   // Truncate the message to fit on display.  You can try scrolling
  if (strcmp(bufferRdsMsg, rdsMsg) == 0)
    return;
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("RDS_megage:     ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rdsMsg);
  delay(5000);
}

/**
   TODO: process RDS Dynamic PS or Scrolling PS
*/
void showRDSStation()
{
  if (strncmp(bufferStatioName, stationName, 3) == 0)
    return;
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("stationName:    ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(stationName);
  delay(5000);
}

void showRDSTime()
{
  if (strcmp(bufferRdsTime, rdsTime) == 0)
    return;
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("RDS_Time:       ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rdsTime);
  delay(5000);
}

void setup() {

  //Serial.begin(9600);
  //while(!Serial);

  //delay(500);
  
/*
  if (!checkI2C())
  {
      Serial.println("\nCheck your circuit!");
      while(1);
  }
*/
  //--------------------------------------------------------
  //--------------------------LCD---------------------------
  lcd.begin(16,2);               // initialize the lcd 
  lcd.createChar (0, smiley);    // load character to the LCD
  lcd.createChar (1, armsUp);    // load character to the LCD
  lcd.createChar (2, frownie);   // load character to the LCD
  lcd.home ();                   // go home
  lcd.print("Hello,   RDA5807");  
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(" FORUM - fm     ");      
  //--------------------------------------------------------
  delay(100);
  //--------------------------------------------------------
  //---------------------RDA5807----------------------------
  rx.setup();
  rx.setVolume(6);
  rx.setFrequency(FM_STATION_FREQ);
  rx.getRdsReady();
  //rx.setVolume(8);  
  //--------------------------------------------------------

  
  //****
  //Serial.print("\nintervolna 102.9MHz");
  //rx.setFrequency(FM_STATION_FREQ); // The frequency you want to select in MHz multiplied by 100.
  
  //Serial.print("\nCurrent Channel: ");
  //Serial.print(rx.getRealChannel());
  //delay(500);

  //Serial.print("\nReal Frequency.: ");
  //Serial.print(rx.getRealFrequency());
  
  //Serial.print("\nRSSI: ");
  //Serial.print(rx.getRssi());

/*
  // Mute test
  Serial.print("\nAfter 5s device will mute during 3s");
  delay(5000);
  rx.setMute(true);
  delay(3000);
  rx.setMute(false);
  Serial.print("\nMute test has finished.");
*/

/*
  // Seek test
  Serial.print("\nSeek station");
  for (int i = 0; i < 10; i++ ) { 
    rx.seek(1,0);
    Serial.print("\nReal Frequency.: ");
    Serial.print(rx.getRealFrequency());
    delay(5000);
  }
*/

//Serial.print("\nIntervolna 102.9MHz");
  //rx.setFrequency(FM_STATION_FREQ);
  
}


void loop() 
{
  // Do a little animation by writing to the same location
  /*
  lcd.setCursor ( 15, 1 );
  lcd.print (char(2));
  delay (300);
  lcd.setCursor ( 15, 1 );
  lcd.print ( char(0));
  delay (300);
  */
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("CHANELL:        ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRealChannel());
  delay(5000);
  lcd.clear();
  lcd.home ();                    // go to the next line
  lcd.print("REAL_FREQ:      ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRealFrequency());
  delay(5000);
  lcd.clear();
  lcd.home ();                  // go to the next line
  lcd.print("RSSI:           ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRssi());
  delay(5000);


  showRDSStation();
  showRDSMsg();
  showRDSTime();

  //Serial.print("\nCurrent Channel: ");
  //Serial.print(rx.getRealChannel());
  //delay(500);

  //Serial.print("\nReal Frequency.: ");
  //Serial.print(rx.getRealFrequency());
  
  //Serial.print("\nRSSI: ");
  //Serial.print(rx.getRssi());

}


/**
 * Returns true if device found
 */

/*
bool checkI2C() {
  Wire.begin();
  byte error, address;
  int nDevices;
  Serial.println("I2C bus Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("\nI2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("\nUnknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    return false;
  }
  else {
    Serial.println("done\n");
    return true;
  }
}
*/