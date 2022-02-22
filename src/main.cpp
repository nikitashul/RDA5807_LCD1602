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

#include <Wire.h>
#include <Arduino.h>
#include <RDA5807.h>
#include <LiquidCrystal.h>

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
#define DEBUG 0
char buffer[30];
unsigned int RDS[4];
char seg_RDS[8];
char seg_RDS1[64];
char indexRDS1;

char hora,minuto,grupo,versio;
unsigned long julian;
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

// Show current frequency
void showStatus()
{
  char aux[80];
  sprintf(aux,"\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | %s ",rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No" );
  Serial.println(aux);
}

//READ RDS  Direccion 0x11 for random access
void ReadW()
{
   Wire.beginTransmission(0x11);            // Device 0x11 for random access
   Wire.write(0x0C);                        // Start at Register 0x0C
   Wire.endTransmission(0);                 // restart condition
   Wire.requestFrom(0x11,8, 1);             // Retransmit device address with READ, followed by 8 bytes
   for (int i=0; i<4; i++) {RDS[i]=256*Wire.read()+Wire.read();}        // Read Data into Array of Unsigned Ints
   Wire.endTransmission();                  
} 

void get_RDS()
{    
  int i;
  ReadW();      
  grupo=(RDS[1]>>12)&0xf;
      if(RDS[1]&0x0800) versio=1; else versio=0;  //Version A=0  Version B=1   
      if(versio==0)
      {
       #if DEBUG             
       sprintf(buffer,"Version=%d  Grupo=%02d ",versio,grupo); Serial.print(buffer);
    //    Serial.print(" 0->");Serial.print(RDS[0],HEX);Serial.print(" 1->");Serial.print(RDS[1],HEX);Serial.print(" 2->");Serial.print(RDS[2],HEX);Serial.print(" 3->");Serial.println(RDS[03],HEX);
    //    Serial.print(" 0->");Serial.print(RDS[0],BIN);Serial.print(" 1->");Serial.print(RDS[1],BIN);Serial.print(" 2->");Serial.print(RDS[2],BIN);Serial.print(" 3->");Serial.println(RDS[03],BIN);
    #endif 
    switch(grupo)
    {
     case 0:              
      #if DEBUG 
      Serial.print("_RDS0__");     
      #endif
      i=(RDS[1] & 3) <<1;
      seg_RDS[i]=(RDS[3]>>8);       
      seg_RDS[i+1]=(RDS[3]&0xFF);
      //gotoXY(10,4);
      for (i=0;i<8;i++)
      {
        #if DEBUG 
        Serial.write(seg_RDS[i]);   
        #endif
        
      }  
      //Serial.print("FrecuAlt1-");Serial.println((RDS[2]>>8)+875);
      //Serial.print("FrecuAlt2-"); Serial.println(RDS[2]&0xFF+875);      
      
      #if DEBUG                 
      Serial.println("---");
      #endif
      break;
     case 2:
      i=(RDS[1] & 15) <<2;              
      seg_RDS1[i]=(RDS[2]>>8);       
      seg_RDS1[i+1]=(RDS[2]&0xFF);
      seg_RDS1[i+2]=(RDS[3]>>8);       
      seg_RDS1[i+3]=(RDS[3]&0xFF);
      #if DEBUG 
      Serial.println("_RADIOTEXTO_");
              //Serial.print(i);Serial.print("   ");Serial.println(RDS[1] & 15);
              //Serial.write(RDS[2]>>8); Serial.write (RDS[2]&0xFF);Serial.write(RDS[3]>>8);Serial.write(RDS[3]&0xFF);Serial.write("_");
              for (i=0;i<32;i++)  Serial.write(seg_RDS1[i]);                                    
              Serial.println("-TXT-");
              #endif      
              break;
              case 4:             
              i=RDS[3]& 0x003f;
              minuto=(RDS[3]>>6)& 0x003f;
              hora=(RDS[3]>>12)& 0x000f;
              if(RDS[2]&1) hora+=16;
              hora+=i;        
              //z=RDS[2]>>1;
              julian = RDS[2]>>1;
              
              if(RDS[1]&1) julian+=32768;
              if(RDS[1]&2) julian+=65536;
              #if DEBUG 
              Serial.print("_DATE_");
              Serial.print(" Juliano=");Serial.print(julian);
              //sprintf(buffer," %02d:%02d ",hora,minuto); gotoXY(38,2);  LcdString(buffer); 
              Serial.println(buffer); 
              #endif            
              break;
              default:
              #if DEBUG 
              Serial.println("__"); 
              #endif    
              ;        
            }                        
    }                   
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
  lcd.begin(16,2);                  // initialize the lcd 
  lcd.createChar (0, smiley);       // load character to the LCD
  lcd.createChar (1, armsUp);       // load character to the LCD
  lcd.createChar (2, frownie);      // load character to the LCD
  lcd.home ();                      // go home
  lcd.print("Hello,   RDA5807");  
  lcd.setCursor ( 0, 1 );           // go to the next line
  lcd.print(" FORUM - fm     ");      
  //--------------------------------------------------------
  delay(100);
  //--------------------------------------------------------
  //-------------------------RDA5807------------------------
  //--------------------------------------------------------
  rx.setup();
  rx.setVolume(6);
  rx.setBass(true);                 // Sets Bass Boost (value	FALSE = Disable; TRUE = Enable)
  rx.setMono(true);                 // Force mono (value	TRUE = Mono; FALSE force stereo)
  // rx.setRBDS(true);              // set RDS and RBDS. See setRDS.
  rx.setRDS(true);                  // set RDS.
  rx.setRdsFifo(true);
  rx.setFrequency(FM_STATION_FREQ); // set STATION Freq.
  //--------------------------------------------------------
  
}

void loop() 
{
  delay(1000);
  //showStatus_on_lcd();
  showStatus();
  delay(1000);
  Serial.println("---------------RDS -- get_RDS() in-------------------");
  //get_RDS();
  ReadW();
  Serial.print("\n");
  for (unsigned char i=0; i<64; i++)
  {
    
    Serial.print(RDS[i]);
  }
  Serial.print("\n");
  Serial.println("---------------RDS -- get_RDS() out------------------");

  Serial.println("---------------RDS -- TIME in-------------------");
  Serial.print("\n");
  Serial.println(RDS[0], BIN);
  Serial.println(RDS[1], BIN);
  Serial.println(RDS[2], BIN);
  Serial.println(RDS[3], BIN);
  Serial.println(RDS[4], BIN);
  Serial.println(RDS[5], BIN);
  Serial.println(RDS[6], BIN);
  Serial.println(RDS[7], BIN);
  Serial.print("\n");
  Serial.println("---------------RDS -- TIME out-------------------");

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