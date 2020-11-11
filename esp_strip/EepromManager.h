#pragma once

#include <EEPROM.h>
#include "Types.h"
#define EEPROM_TOTAL_BYTES_USED              (82U)         // общий размер используемой EEPROM памяти (сумма всех хранимых настроек + 1 байт)
#define EEPROM_MODES_START_ADDRESS           (0U)          // начальный адрес в EEPROM памяти для записи настроек эффектов (яркость, скорость, масштаб)
#define EEPROM_LAMP_ON_ADDRESS               (71U)         // адрес в EEPROM памяти для записи состояния лампы (вкл/выкл)
#define EEPROM_FIRST_RUN_ADDRESS             (73U)         // адрес в EEPROM памяти для записи признака первого запуска (определяет необходимость первоначальной записи всех хранимых настроек)
#define EEPROM_CURRENT_MODE_ADDRESS          (74U)         // адрес в EEPROM памяти для записи номера текущего эффекта лампы
#define EEPROM_LAMP_ID_ADRESS                (75U)
#define EEPROM_LOW_PASS_ADRESS               (77U)
#define EEPROM_REACTION_ADRESS               (80U)
#define EEPROM_RAVE_MODE_ADRESS              (81U)

#define EEPROM_MODE_STRUCT_SIZE              (5U)           // 1 байт - яркость; 1 байт - скорость; 1 байт - масштаб

#define EEPROM_FIRST_RUN_MARK                (24U)          // число-метка, если ещё не записно в EEPROM_FIRST_RUN_ADDRESS, значит нужно проинициализировать EEPROM и записать все первоначальные настройки
#define EEPROM_WRITE_DELAY                   (30000UL)      // отсрочка записи в EEPROM после последнего изменения хранимых настроек, позволяет уменьшить количество операций записи в EEPROM


class EepromManager
{
  public:
    static void InitEepromSettings(ModeType modes[], uint8_t* espMode, bool* onFlag, int8_t* currentMode, bool* buttonEnabled)
    {
      // записываем в EEPROM начальное состояние настроек, если их там ещё нет
      EEPROM.begin(EEPROM_TOTAL_BYTES_USED);
      delay(50);

      if (EEPROM.read(EEPROM_FIRST_RUN_ADDRESS) != EEPROM_FIRST_RUN_MARK)
      {
        EEPROM.write(EEPROM_FIRST_RUN_ADDRESS, EEPROM_FIRST_RUN_MARK);
        EEPROM.commit();

        for (uint8_t i = 0; i < MODE_AMOUNT; i++)
        {
          EEPROM.put(EEPROM_MODES_START_ADDRESS * i, modes[i]);
          EEPROM.commit();
        }


        EEPROM.write(EEPROM_LAMP_ON_ADDRESS, 0);
        EEPROM.write(EEPROM_CURRENT_MODE_ADDRESS, 0);
        EEPROM.write(EEPROM_REACTION_ADRESS, 0);
        EEPROM.write(EEPROM_RAVE_MODE_ADRESS, 0);
            
        EEPROM.commit();
      }

      // инициализируем настройки лампы значениями из EEPROM
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        EEPROM.get(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
        //Serial.print("EEPROM_MODES_");
        //Serial.print(i);
        //Serial.println(": ");

        //Serial.print("BRIGHTNESS ");
        //Serial.println(modes[i].Brightness);

        //Serial.print("BG BRIGHTNESS ");
        //Serial.println(modes[i].BGBrightness);

        //Serial.print("SPEED ");
        //Serial.println(modes[i].Speed);

        //Serial.print("HUE ");
        //Serial.println(modes[i].Color[0]);

        //Serial.print("SATURATION ");
        //Serial.println(modes[i].Color[1]);
        
        //Serial.println();
      }
        //Serial.print("EEPROM_LAMP_ON: ");
        //Serial.println(EEPROM.read(EEPROM_LAMP_ON_ADDRESS));
        //Serial.print("EEPROM_CURRENT_MODE: ");
        //Serial.println(EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS));
        //Serial.print("EEPROM_LAMP_ID: ");
        //Serial.println(EEPROM.read(EEPROM_LAMP_ID_ADRESS));
        //Serial.print("EEPROM_LOW_PASS: ");
        //Serial.println(EEPROM.read(EEPROM_LOW_PASS_ADRESS));
        //Serial.print("EEPROM_REACTION: ");
        //Serial.println(EEPROM.read(EEPROM_REACTION_ADRESS));
        //Serial.print("EEPROM_RAVE_MODE: ");
        //Serial.println(EEPROM.read(EEPROM_RAVE_MODE_ADRESS));

      *onFlag = (bool)EEPROM.read(EEPROM_LAMP_ON_ADDRESS);
      *currentMode = EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS);
      REACTION = EEPROM.read(EEPROM_REACTION_ADRESS);
      RAVE_MODE = EEPROM.read(EEPROM_RAVE_MODE_ADRESS);
      LOW_PASS = EEPROM.read(EEPROM_LOW_PASS_ADRESS);
    }

    static void SaveModesSettings(int8_t* currentMode, ModeType modes[])
    {
      EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * (*currentMode), modes[*currentMode]);
      EEPROM.commit();
    }

    static void SaveReaction(bool reaction)
    {
      EEPROM.write(EEPROM_REACTION_ADRESS, reaction);
      EEPROM.commit();
    }

    static void SaveRaveMode(bool raveMode)
    {
      EEPROM.write(EEPROM_RAVE_MODE_ADRESS, raveMode);
      EEPROM.commit();
    }

    static void SaveID()
    {
      #ifdef LAMP_ID
      EEPROM.write(EEPROM_LAMP_ID_ADRESS, LAMP_ID);
      #endif
    }

    static int GetID()
    {
      #ifdef LAMP_ID
      return LAMP_ID;
      #else
      return EEPROM.read(EEPROM_LAMP_ID_ADRESS);
      #endif
    }
    
    static void HandleEepromTick(bool* settChanged, uint32_t* eepromTimeout, bool* onFlag, int8_t* currentMode, ModeType modes[])
    {
      if (*settChanged && millis() - *eepromTimeout > EEPROM_WRITE_DELAY)
      {
        *settChanged = false;
        *eepromTimeout = millis();
        SaveOnFlag(onFlag);
        SaveModesSettings(currentMode, modes);
        if (EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS) != *currentMode)
        {
          EEPROM.write(EEPROM_CURRENT_MODE_ADDRESS, *currentMode);
        }
        EEPROM.commit();
      }
    }

    static void SaveOnFlag(bool* onFlag)
    {
      EEPROM.write(EEPROM_LAMP_ON_ADDRESS, *onFlag);
      EEPROM.commit();
    }

    static uint16_t ReadUint16(uint16_t address)
    {
      uint16_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      return val;
    }

    static void WriteUint16(uint16_t address, uint16_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.commit();
    }

    static int16_t ReadInt16(uint16_t address)
    {
      int16_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      return val;
    }

    static void WriteInt16(uint16_t address, int16_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.commit();      
    }

    static uint32_t ReadUint32(uint16_t address)
    {
      uint32_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      *(p + 2)  = EEPROM.read(address + 2);
      *(p + 3)  = EEPROM.read(address + 3);
      return val;
    }

    static void WriteUint32(uint16_t address, uint32_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.write(address + 2, *(p + 2));
      EEPROM.write(address + 3, *(p + 3));
      EEPROM.commit();
    }

    static int32_t ReadInt32(uint16_t address)
    {
      int32_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      *(p + 2)  = EEPROM.read(address + 2);
      *(p + 3)  = EEPROM.read(address + 3);
      return val;
    }

    static void WriteInt32(uint16_t address, int32_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.write(address + 2, *(p + 2));
      EEPROM.write(address + 3, *(p + 3));
      EEPROM.commit();      
    }

  private:
};
