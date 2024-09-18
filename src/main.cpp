/*
 Надо глянуть проект
 https://github.com/sdg12832/sdg12832/blob/master/Radio.ino#L88
*/


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
    |     RDA5807               | ------------------------- |               |
    |                           | SDA (pin 1)               |     A4        |
    |                           | SCK (pin 2)               |     A5        |
    |     Buttons               | ------------------------- |               |
    |                           | Volume Up                 |               |
    |                           | Volume Down               |               |
    |                           | Stereo/Mono               |               |
    |                           | RDS ON/off                |               |
    |                           | SEEK (encoder button)     |               |
    |     Encoder               | ------------------------- |               |
    |                           | A                         |               |
    |                           | B                         |               |
    |     Button                | KEY 1   +                 |      7        | Подтяжка к +5
    |     Button                | KEY 2   -                 |      8        | Подтяжка к +5
    |     Button                | SET                       |      6        | Подтяжка к +5
*/

#include <Wire.h>
#include <Arduino.h>
#include <RDA5807.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define DEBUG_INIT 0 //Debug on/off

#define RS 12
#define EN 11
#define D0 5
#define D1 4
#define D2 3
#define D3 2

#define KEY_1   7
#define KEY_2   8
#define KEY_SET 6

RDA5807 rx; 

uint16_t PROGRAMMS[] = {  
  8790,  // 87.90 Monte Carlo
  9260,  // Vesti FM
  9300,  // Radio Gordost
  9360,  // Mayak
  9400,  // Mir
  9460,  // Love Radio
  9530,  // Koms Pravda
  9600,  // 96.00 ENERGY
  9640,  // Retro FM
  9780,  // Russia
  9870,  // Dacha
  9910,  // Avtoradio
  10000, //100.00 Радио 100
  10040, //100.4 Kontinental
  10080, //100.80 Bisness
  10120, //101.20 Yumor FM
  10160, //101.60 Europa+
  10200, //102.00 Novoe
  10240, //102.4 Studio 21
  10290, //102.9 Intervolna
  10350, //103.50 Nawe
  10410, //104.10 Russkoe
  10450, //104.50 Olimp
  10490, //104.90 L-radio
  10540, //7 na 7 xolmax
  10590, //Shanson
  10630, //Dorognoe
  10680, //Vania
  10730, //DFM
  10780  //Comedy                        
                        
                        };

/*
  char *s = "text";  до / C++11 /
  const char *s = "text";  начиная / C++11 /
  Объявление указателя s типа char * и присваивание ему указателя на первый элемент строкового литерала "text". Попытка изменить этот строковой литерал (s[0] = 'a', например), — это неопределенное поведение.

  Однако сам указатель переприсваивать можно:

  const char *p = "text";  / OK /
  p = "another text";      / OK /
  Начиная с C++11 строковые литералы могут быть прямо присвоены только const char * (т. е. только указателям на константный char).
*/
const char* PROGRAMMS_LABEL[] = {   "Monte Carlo     ",
                              "Vesti FM        ",
                              "Radio Gordost   ",
                              "Mayak           ",
                              "Mir             ",
                              "Love Radio      ",
                              "Koms Pravda     ",
                              "ENERGY          ",
                              "Retro FM        ",
                              "Russia          ",
                              "Dacha           ",
                              "Avtoradio       ",
                              "Radio 100       ",
                              "Kontinental     ",
                              "Bisiness FM     ",
                              "Yumor FM        ",
                              "Europa+         ",
                              "NOVOE           ",
                              "STUDIO 21       ",
                              "Intervolna      ",
                              "NASHE RADIO     ",
                              "Russkoe Radio   ",
                              "Radio Olimp     ",
                              "L-RADIO         ",
                              "Radio 7         ",
                              "SHanson         ",
                              "Dorognoe        ",
                              "Vania           ",
                              "DFM             ",
                              "Comedi          "
                        };

unsigned char SET_STATION = EEPROM.read(0); //Считываем какую станцию сохранили.

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
   Делаем "параллельное" выполнение нескольких задач
   с разным периодом выполнения
*/
#define PERIOD_1 300    // период //Вывод данных на LCD1602
#define PERIOD_2 15     // период //Период опроса кнопок

unsigned long timer_1, timer_2;                               //Переменные для реализации таймеров
uint8_t KEY_1_count = 0, KEY_2_count = 0, KEY_SET_count = 0;  //Счетчики для антидребезга
//bool KEY_1_flag = 0, KEY_2_flag = 0, KEY_SET_flag = 0;      //Убрать неиспользуется

void TIMER_TIC();         //Функция обрабатывает таймеры для выполнения задач.
void showStatus_on_lcd(); //Функция обрабатывает вывод информации на LCD1602
void BUTTON();            //Функция обрабатывает нажатия кнопок
bool ANTIBOUNCE(uint8_t KEY, uint8_t &COUNT);        //Функция антидребезг

void setup() {
  //Очистка памяти при первом программировании
  //EEPROM.write(0, 0);
  //--------------------------------------------------------
  //-------------------------USART--------------------------
  //--------------------------------------------------------
  Serial.begin(9600);

  pinMode(KEY_1, INPUT);       // устанавливает режим работы - вход
  pinMode(KEY_2, INPUT);       // устанавливает режим работы - вход
  pinMode(KEY_SET, INPUT);     // устанавливает режим работы - вход
  digitalWrite(KEY_1, HIGH);   //Включаем подтяжку к +5В
  digitalWrite(KEY_2, HIGH);   //Включаем подтяжку к +5В
  digitalWrite(KEY_SET, HIGH); //Включаем подтяжку к +5В

  //--------------------------------------------------------
  
  //--------------------------------------------------------
  //--------------------------LCD---------------------------
  //--------------------------------------------------------
  lcd.begin(16,2);                  // initialize the lcd 
      
  if (DEBUG_INIT){
    Serial.println("DEBUG INIT ON");
    Serial.println("Serial.begin(9600); ------ OK");
    lcd.createChar (0, smiley);       // load character to the LCD
    lcd.createChar (1, armsUp);       // load character to the LCD
    lcd.createChar (2, frownie);      // load character to the LCD
    delay(5000);
    lcd.home ();                      // go home
    lcd.print("Hello,   RDA5807");  
    lcd.setCursor ( 0, 1 );           // go to the next line
    lcd.print(" -----TEST----- ");    

    Serial.println("lcd.begin();        ------ OK");
    delay(1000);
  }
  //--------------------------------------------------------
 
  //--------------------------------------------------------
  //-------------------------RDA5807------------------------
  //--------------------------------------------------------
  rx.setup();
  rx.setVolume(4);
  rx.setBass(true);                 // Sets Bass Boost (value	FALSE = Disable; TRUE = Enable)
  rx.setMono(true);                 // Force mono (value	TRUE = Mono; FALSE force stereo)
  rx.setRDS(false);                 // set RDS.
  rx.setRBDS(false);                // set the RBDS operation.
  rx.setRdsFifo(false);
  rx.setFmDeemphasis(true);         //de	0 = 75 μs; 1 = 50 μs (75 μs. Used in USA (default); 50 μs. Used in Europe, Australia, Japan.)
  rx.setFrequency(PROGRAMMS[SET_STATION]);    // set STATION Freq.
  
  if (DEBUG_INIT){
    Serial.print("EEPROM.read(0);     ---- i= ");
    Serial.println(SET_STATION);
    Serial.print("PROGRAMS LABEL;             ");
    Serial.println(PROGRAMMS_LABEL[SET_STATION]);

    Serial.println("RDA5807.setup(); RDA5807 ------ OK");
    delay(1000);
  }
  //--------------------------------------------------------
  
  
}

void loop() 
{
  //Все действия обрабатываются по таймеру millis()
  TIMER_TIC();
}

//Функция обрабатывает таймеры для выполнения задач.
void TIMER_TIC()
{
  if (millis() - timer_1 > PERIOD_1) {    // условие таймера
    timer_1 = millis();                   // сброс таймера
    // выполняем блок №1 каждые PERIOD_1 миллисекунд
    // Индикация в ФУНКЦИИ LCD 16*2
    showStatus_on_lcd();
  }  
  if (millis() - timer_2 > PERIOD_2) {
    timer_2 = millis();
    // выполняем блок №2 каждые PERIOD_2 миллисекунд
    // Обработка нажатия кнопок
    BUTTON();
  }
}

//Функция обрабатывает нажатия кнопок
void BUTTON()
{
  //Serial.print("KEY_1_count: ");
  //Serial.println(KEY_1_count);
  if(ANTIBOUNCE(KEY_1, KEY_1_count))      //Функция возврвщает true если кнопка нажата 
      {
        if (SET_STATION<29){
          rx.setFrequency(PROGRAMMS[++SET_STATION]);   // set STATION Freq.
        }
      }

  if(ANTIBOUNCE(KEY_2, KEY_2_count)==true)      //Функция возврвщает true если кнопка нажата
      {
        if (SET_STATION>0){
          rx.setFrequency(PROGRAMMS[--SET_STATION]);   // set STATION Freq.
        }
        
      }
  if(ANTIBOUNCE(KEY_SET, KEY_SET_count)==true)  //Функция возврвщает true если кнопка нажата
    {
      EEPROM.update(0, SET_STATION);
      lcd.clear();
      lcd.home ();                   // go to the home
      lcd.print("EEPROM SET:        ");
      lcd.setCursor ( 0, 1 );        // go to the next line
      lcd.print(SET_STATION);
      Serial.println("EEPROM SET: ");
      Serial.print(SET_STATION);
      delay(5000);
    }
    

  /*
  //---------------KEY_1---------------------
  //-------------Антидребезг-----------------
  //Проверка, если нажата кнопка и не прошло 10 периодов то инкрементируем (10*10мсек)
  if(digitalRead(KEY_1) == LOW && KEY_1_count<10){
    KEY_1_count ++; 
    if(KEY_1_count >= 10)
    {
      if(SET_STATION <= 10)
      {
        rx.setFrequency(PROGRAMMS[++SET_STATION]);   // set STATION Freq.
      }
    }
  }
  //Сброс состояния кнопки
  if(digitalRead(KEY_1) == HIGH && KEY_1_count > 0)
  {
    KEY_1_count--;
  }

  //---------------KEY_2---------------------
  //-------------Антидребезг-----------------
  //Проверка, если нажата кнопка и не прошло 10 периодов то инкрементируем (10*10мсек)
  if(digitalRead(KEY_2) == LOW && KEY_2_count<10){
    KEY_2_count ++; 
    if(KEY_2_count >= 10)
    {
      if(SET_STATION > 0)
      {
        rx.setFrequency(PROGRAMMS[--SET_STATION]);   // set STATION Freq.
      }
    }
  }
  //Сброс состояния кнопки
  if(digitalRead(KEY_2) == HIGH && KEY_2_count > 0)
  {
    KEY_2_count--;
  }

  //---------------KEY_SET---------------------
  //Проверка, если нажата кнопка и не прошло 10 периодов то инкрементируем (10*10мсек)
  if(digitalRead(KEY_SET) == LOW && KEY_SET_count<10){
    KEY_SET_count ++; 
    if(KEY_SET_count >= 10)
    {
      EEPROM.update(0, SET_STATION);
      lcd.clear();
      lcd.home ();                   // go to the home
      lcd.print("EEPROM SET:        ");
      lcd.setCursor ( 0, 1 );        // go to the next line
      lcd.print(SET_STATION);
      Serial.println("EEPROM SET: ");
      Serial.print(SET_STATION);
      //delay(2000);
    }
  }
  //Сброс состояния кнопки
  if(digitalRead(KEY_SET) == HIGH && KEY_SET_count > 0)
  {
    KEY_SET_count--;
  }
  */
}

//Функция антидребезг
bool ANTIBOUNCE(uint8_t KEY, uint8_t &COUNT)      
{
  //-------------Антидребезг-----------------
  //Проверка, если нажата кнопка и не прошло 10 периодов то инкрементируем (10*15мсек)
  //Serial.println("ANTIBOUNCE(KEY_1, KEY_1_count)");
  bool x = 0;
  /*
  Serial.println(KEY);
  Serial.println(COUNT);
  delay(1000);
  */
  if(digitalRead(KEY) == LOW && COUNT<10){
    //Serial.println(COUNT);
    COUNT ++;
    if(COUNT >= 10)
    {
      //CLIC BUTTON
      x = true;
    }
    else{
      x = false;
    }
    
  }
  //Сброс состояния кнопки
  if(digitalRead(KEY) == HIGH && COUNT > 0)
  {
    COUNT --;
  
  }
  //Возвращаю значение
  
  return x;

}

//Функция обрабатывает вывод информации на LCD1602
void showStatus_on_lcd()
{
  lcd.clear();
  lcd.home ();                   // go to the home
  lcd.print("FREQ: ");
  lcd.print(rx.getRealFrequency());
  lcd.setCursor ( 0, 1 );        // go to the next line
  //lcd.print("RSSI: ");
  //lcd.print(rx.getRssi());
  lcd.print(PROGRAMMS_LABEL[SET_STATION]);
  
  if (DEBUG_INIT){
    Serial.print("CHANELL: ");
    Serial.println(rx.getRealChannel());
    Serial.print("REAL_FREQ: ");
    Serial.println(rx.getRealFrequency());
    Serial.print("RSSI: ");
    Serial.println(rx.getRssi());
    Serial.print("LABEL: ");
    Serial.println(PROGRAMMS_LABEL[8]);
    //Serial.println(ANTIBOUNCE(KEY_1, KEY_1_count));
    //delay(1000);
    
  }
  
}