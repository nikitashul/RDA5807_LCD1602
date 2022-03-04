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

#define DEBUG_INIT 1

RDA5807 rx; 

//unsigned int FM_STATION_FREQ = 10450; //104.50 Olimp
unsigned int FM_STATION_FREQ = 10290; //102.90 Intervolna

#define   CONTRAST_PIN   9
#define   BACKLIGHT_PIN  7
#define   CONTRAST       110

/*
bool bSt = true;
bool bRds = true;
bool bShow = false;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.
*/

//-------------------RDS-------------------
/*
#define POLLING_TIME  2000
#define POLLING_RDS     80

char oldFreq[10];
char oldStereo[10];
char oldRssi[10];
char oldRdsStatus[10];
char oldRdsMsg[65];

bool bSt = true;
bool bRds = true;
bool bShow = false;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.

long pollin_elapsed = millis();
*/
//-----------------------------------------

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

//----------------rx.getRdsProgramType()------------------
// ---PTy код	Тип контента RDS
//char имя[количество][длина]	
char ProgramType [][22] = {"No type",	"News",
                              "Current affairs",
                              "Information",
                              "Sport",
                              "Education",
                              "Drama",
                              "Culture",
                              "Science",
                              "Varied",
                              "Pop music",
                              "Rock music",
                              "Easy listening",
                              "Light classical",
                              "Serious classical",
                              "Other music",
                              "Weather",
                              "Finance",
                              "Childrens programmes",
                              "Social affairs",
                              "Religion",
                              "Phone-in",
                              "Travel",
                              "Leisure",
                              "Jazz music",
                              "Country music",
                              "National music",
                              "Oldies music",
                              "Folk music",
                              "Documentary",
                              "Alarm test",
                              "Alarm"
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
  lcd.print(" FORUM - fm     ");      
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
  rx.setRDS(true);                  // set RDS.
  rx.setRBDS(true);                 // set the RBDS operation.
  rx.setRdsFifo(true);
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
  //delay(2000);
  //showStatus_on_lcd();
  //Serial.println("---------------RDS -- get_RDS() in-------------------");
  
  if(rx.getRdsSync()){    //С этого момента RDS декодер синхронизирован. Данный факт отражается установкой флага RDSS регистра 0AH
    //Serial.println("С этого момента RDS декодер синхронизирован");
    if(rx.getRdsReady()){ // "Информация получена" 
      //Serial.println("Информация получена");
      if( rx.hasRdsInfo()){  // Если достоверная доставерная (rx.hasRdsInfo()==true)    
          //Serial.println("Информация достоверная");
          
          //
          /*
          Serial.print("rx.getRdsProgramType(): ");
          for (unsigned char i=0; i<22; i++)
          {
            Serial.print(ProgramType [rx.getRdsProgramType()][i]);
          }
          */
          Serial.println();
          //Serial.print(ProgramType [rx.getRdsProgramType()][0]);
          //Serial.println("RdsProgram: ");
          //Serial.println(rx.getRdsProgramType());
          
          //Serial.print("rx.getRdsTime(): ");
          //Serial.println(rx.getRdsTime());
          //Serial.println("RdsText():  ");
          /*
          Serial.println(rx.getRdsText0A());
          Serial.println(rx.getRdsText2A());
          Serial.println(rx.getRdsText2B());
          Serial.println(rx.getRdsTime());
          Serial.print("ErrorBlockB: ");
          Serial.println(rx.getErrorBlockB());
          */
          //Serial.print("rx.hasRdsInfo(): ");
          //Serial.println(rx.hasRdsInfo());
          //delay(1000);
          Serial.println(rx.getRdsGroupType());
          Serial.println(rx.getRdsVersionCode()); // 0 - A, 1- B
          if(rx.getRdsVersionCode()){ //Если B (1- B)
            Serial.print("RdsProgramType: ");
            Serial.println(rx.getRdsProgramType());
          }
          Serial.println(rx.getRealFrequency()); //Возвращает реальную частоту приемника
          Serial.println(rx.getRssi(), BIN); //RSSI; 000000 = min; 111111 = max; RSSI scale is logarithmic.
          Serial.println(rx.getRssi(), DEC);
          Serial.println(rx.getRdsText0A());
          Serial.println(rx.getRdsText2A());
          Serial.println(rx.getRdsText2B());
          delay(1000);
        }
      //rx.clearRdsFifo();
    }

  }


}