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

//unsigned int FM_STATION_FREQ = 10450; //104.50 Olimp
unsigned int FM_STATION_FREQ = 10290; //102.90 Intervolna


#define   CONTRAST_PIN   9
#define   BACKLIGHT_PIN  7
#define   CONTRAST       110

bool bSt = true;
bool bRds = true;
bool bShow = false;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.

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
  delay(5000);
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("REAL_FREQ:      ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRealFrequency());
  delay(5000);
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("RSSI:           ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRssi());
  delay(5000);
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("RdsProgramType: ");
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print(rx.getRdsProgramType());
  delay(5000);

  //showFrequency();
  //showStereoMono();
  //showRSSI();
}

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
  Serial.print("RDS_MEGAGE: ");
  Serial.println(rdsMsg);
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
  Serial.print("RDS_STATION_NAME: ");
  Serial.println(stationName);
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
  Serial.print("RDS_TIME: ");
  Serial.println(rdsTime);
  delay(5000);
}

void clearRds() {
  //lcd.clear();
  bShow = false;
}

void checkRDS()
{
  // check if RDS currently synchronized; the information are A, B, C and D blocks; and no errors
  if ( rx.hasRdsInfo() ) {
    rdsMsg = rx.getRdsText2A();
    stationName = rx.getRdsText0A();
    rdsTime = rx.getRdsTime();
    if (rdsMsg != NULL)
      showRDSMsg();

    if ((millis() - stationNameElapsed) > 1000)
    {
      if (stationName != NULL)
        showRDSStation();
      stationNameElapsed = millis();
    }

    if (rdsTime != NULL)
      showRDSTime();
  }

  if ( (millis() - clear_fifo) > 10000 ) {
    rx.clearRdsFifo();
    clear_fifo = millis();
    
  }
}

void showRds() {
  char rdsStatus[10];

  sprintf(rdsStatus, "RDS %s", (bRds) ? "ON" : "OFF");
  Serial.print("RDS_STATUS: ");
  Serial.println(rdsStatus);
  checkRDS();
}

void setup() {
  //--------------------------------------------------------
  //-------------------------USART--------------------------
  //--------------------------------------------------------
  Serial.begin(9600);
  //--------------------------------------------------------

  //--------------------------------------------------------
  //--------------------------LCD---------------------------
  //--------------------------------------------------------
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
  //-------------------------RDA5807------------------------
  //--------------------------------------------------------
  rx.setup();
  rx.setVolume(6);
  rx.setMono(true);     // Force mono
  // rx.setRBDS(true);  // set RDS and RBDS. See setRDS.
  rx.setRDS(true);      // set RDS.
  rx.setRdsFifo(true);
  rx.setFrequency(FM_STATION_FREQ);
  //--------------------------------------------------------
  
}

void loop() 
{
  showRds();
  delay(3000);
  showStatus_on_lcd();
  delay(3000);

/*
  if(rx.getRdsSync()){ //С этого момента RDS декодер синхронизирован. Данный факт отражается установкой флага RDSS регистра 0AH
    Serial.println("С этого момента RDS декодер синхронизирован");
    if(rx.getRdsReady()){
      Serial.println("Информация получена");
      Serial.print("rx.getRdsProgramType(): ");
      Serial.println(rx.getRdsProgramType());
      delay(3000);
      Serial.print("rx.getRdsTime(): ");
      Serial.println(rx.getRdsTime());
      delay(3000);
      Serial.print("rx.getRdsText(): ");
      Serial.println(rx.getRdsText());
      delay(3000);
      Serial.print("rx.hasRdsInfo(): ");
      Serial.println(rx.hasRdsInfo());
      delay(3000);
    }
  }
*/

}
  
  //showRDSStation();
  //showRDSMsg();
  //showRDSTime();

