/*
    | Device name               | Device Pin / Description  |  Arduino Pin  |
    | ----------------          | --------------------      | ------------  |
    |    Display LCD1602        |                           |               |
    |                           | RST (RESET)               |      8        |
    |                           | RS  or DC                 |      9        |
    |                           | CS  or SS                 |     10        |
    |                           | SDI                       |     11        |
    |                           | CLK                       |     13        |
    |     RDA5807               |                           |               |
    |                           | SDIO (pin 8)              |     A4        |
    |                           | SCLK (pin 7)              |     A5        |
    |     Buttons               |                           |               |
    |                           | Volume Up                 |               |
    |                           | Volume Down               |               |
    |                           | Stereo/Mono               |               |
    |                           | RDS ON/off                |               |
    |                           | SEEK (encoder button)     |               |
    |    Encoder                |                           |               |
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

LiquidCrystal lcd(12, 11, 5, 4, 3, 2, BACKLIGHT_PIN, POSITIVE );


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

void setup() {

  Serial.begin(9600);
  while(!Serial);

  delay(500);
  
/*
  if (!checkI2C())
  {
      Serial.println("\nCheck your circuit!");
      while(1);
  }
*/

  rx.setup();
  rx.setVolume(6);
  //rx.setVolume(8);  

  
  delay(500);
  //****
  Serial.print("\nintervolna 102.9MHz");
  rx.setFrequency(FM_STATION_FREQ); // The frequency you want to select in MHz multiplied by 100.
  
  Serial.print("\nCurrent Channel: ");
  Serial.print(rx.getRealChannel());
  delay(500);

  Serial.print("\nReal Frequency.: ");
  Serial.print(rx.getRealFrequency());
  
  Serial.print("\nRSSI: ");
  Serial.print(rx.getRssi());

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
  rx.setFrequency(FM_STATION_FREQ);
  
}


void loop() 
{

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