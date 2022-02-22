#include <Wire.h>

#define RDA5807M_RANDOM_ACCESS_ADDRESS 0x11
// регистры
#define RDA5807M_REG_CONFIG 0x02
#define RDA5807M_REG_TUNING 0x03
#define RDA5807M_REG_VOLUME 0x05
#define RDA5807M_REG_STATUS1 0x0A
#define RDA5807M_REG_STATUS2 0x0B
#define RDA5807M_REG_BLER_CD 0x10
#define RDA5807M_REG_BLOCK_A 0x0C
#define RDA5807M_REG_BLOCK_B 0x0D
#define RDA5807M_REG_BLOCK_C 0x0E
#define RDA5807M_REG_BLOCK_D 0x0F
// флаги
#define RDA5807M_FLG_DHIZ 0x8000
#define RDA5807M_FLG_DMUTE 0x4000
#define RDA5807M_FLG_SEEK 0x0100
#define RDA5807M_FLG_SEEKUP 0x0200
#define RDA5807M_FLG_ENABLE word(0x0001)
#define RDA5807M_FLG_TUNE word(0x0010)
#define RDA5807M_FLG_RDS word(0x0008)
#define RDA5807M_FLAG_RDSR 0x8000
#define RDA5807M_FLAG_STC 0x4000
//маски
#define RDA5807M_CHAN_MASK 0xFFC0
#define RDA5807M_CHAN_SHIFT 6
#define RDA5807M_VOLUME_MASK word(0x000F)
#define RDA5807M_VOLUME_SHIFT 0
#define RDA5807M_BLERB_MASK word(0x0003)

#define RDS_ALL_GROUPTYPE_MASK 0xF000
#define RDS_ALL_GROUPTYPE_SHIFT 12
#define RDS_ALL_GROUPVER 0x800

#define RDSR_CHECK_INTERVAL 30  // Интервал опроса флага RDSR
unsigned long RDSCheckTime = 0; // Предыдущее время опроса RDSR
bool RDS_ready = false;         // Предыдущее значение RDSR
uint16_t ID = 0;                // ID радиостанции
uint16_t MaybeThisIDIsReal = 0; // Предыдущее значение ID
uint8_t IDRepeatCounter = 0;    // Счетчик повторений ID
#define REPEATS_TO_BE_REAL_ID 3 // Количество повторений чтобы признать ID корректным

void setup() {
  uint16_t r;
  Serial.begin(9600);
  Wire.begin();
  //Включаем приемник, разрешаем прием RDS, настраиваемся на следующую радиостанцию
  setRegister(0x02, RDA5807M_FLG_ENABLE | RDA5807M_FLG_DHIZ | RDA5807M_FLG_DMUTE |
                    RDA5807M_FLG_SEEK | RDA5807M_FLG_SEEKUP | RDA5807M_FLG_RDS );
  
  do // Ждем пока приемник найдет станцию
    r = getRegister(RDA5807M_REG_STATUS1);
  while (!(r & RDA5807M_FLAG_STC));
  
  setRegister(RDA5807M_REG_VOLUME, 0x86C0); // Убавим громкость
  
  // Покажем текущую частоту
  r = getRegister(RDA5807M_REG_TUNING);
  Serial.println(87 + double((r & RDA5807M_CHAN_MASK) >> RDA5807M_CHAN_SHIFT) / 10);
}

void loop() {
  uint16_t reg0Ah, reg0Bh;
  uint16_t blockA, blockB, blockC, blockD;
  uint8_t errLevelB, groupType, groupVer;
  char buf[20];
  if (millis() - RDSCheckTime >= RDSR_CHECK_INTERVAL) {
    // Пора проверить флаг RDSR
    RDSCheckTime = millis();
    reg0Ah = getRegister(RDA5807M_REG_STATUS1);
    if ((reg0Ah & RDA5807M_FLAG_RDSR) and (!RDS_ready)) { // RDSR изменился с 0 на 1
      blockA = getRegister(RDA5807M_REG_BLOCK_A);
      blockB = getRegister(RDA5807M_REG_BLOCK_B);
      blockC = getRegister(RDA5807M_REG_BLOCK_C);
      blockD = getRegister(RDA5807M_REG_BLOCK_D);
      
      // Сравним содержимое блока A (ID станции) с предыдущим значением
      if (blockA == MaybeThisIDIsReal) {
        if (IDRepeatCounter < REPEATS_TO_BE_REAL_ID) {
          IDRepeatCounter++; // Значения совпадают, отразим это в счетчике
          if (IDRepeatCounter == REPEATS_TO_BE_REAL_ID)
            ID = MaybeThisIDIsReal; // Определились с ID станции
        }
      }
      else {
        IDRepeatCounter = 0; // Значения не совпадают, считаем заново
        MaybeThisIDIsReal = blockA;
      }
      // Выведем значения блоков
      Serial.print("RDS raw data: ");
      sprintf(buf, "%04X %04X %04X %04X", blockA, blockB, blockC, blockD);
      Serial.print(buf);
      
      //if (ID == 0) return; // Пока не определимся с ID, разбирать RDS не будем
      //if (blockA != ID) return; // ID не совпадает. Пропустим эту RDS группу
      
      // ID станции не скачет, вероятность корректности группы в целом выше
      reg0Bh = getRegister(RDA5807M_REG_STATUS2);
      errLevelB = (reg0Bh & RDA5807M_BLERB_MASK);
      if (errLevelB < 3) {
        // Блок B корректный, можем определить тип и версию группы
        groupType = (blockB & RDS_ALL_GROUPTYPE_MASK) >> RDS_ALL_GROUPTYPE_SHIFT;
        groupVer = (blockB & RDS_ALL_GROUPVER) > 0;
        Serial.print(" Group: ");
        Serial.print(groupType);
        Serial.print(char('A' + groupVer));
      }
      if ((ID == 0) or (blockA != ID)) Serial.print(" - invalid group");
      Serial.println("");
    }
    RDS_ready = reg0Ah & RDA5807M_FLAG_RDSR;
  }
}

void setRegister(uint8_t reg, const uint16_t value) {
  Wire.beginTransmission(0x11);
  Wire.write(reg);
  Wire.write(highByte(value));
  Wire.write(lowByte(value));
  Wire.endTransmission(true);
}

uint16_t getRegister(uint8_t reg) {
  uint16_t result;
  Wire.beginTransmission(RDA5807M_RANDOM_ACCESS_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(0x11, 2, true);
  result = (uint16_t)Wire.read() << 8;
  result |= Wire.read();
  return result;
}