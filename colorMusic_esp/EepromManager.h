#include <EEPROM.h>
#include "Types.h"
#define EEPROM_TOTAL_BYTES_USED              (103U)         // общий размер используемой EEPROM памяти (сумма всех хранимых настроек + 1 байт)
#define EEPROM_MODES_START_ADDRESS           (0U)          // начальный адрес в EEPROM памяти для записи настроек эффектов (яркость, скорость, масштаб)
#define EEPROM_ESP_MODE                      (46U)         // адрес в EEPROM памяти для записи режима работы модуля ESP (точка доступа/WiFi клиент)
#define EEPROM_LAMP_ON_ADDRESS               (47U)         // адрес в EEPROM памяти для записи состояния лампы (вкл/выкл)
#define EEPROM_FIRST_RUN_ADDRESS             (48U)         // адрес в EEPROM памяти для записи признака первого запуска (определяет необходимость первоначальной записи всех хранимых настроек)
#define EEPROM_CURRENT_MODE_ADDRESS          (50U)         // адрес в EEPROM памяти для записи номера текущего эффекта лампы
#define EEPROM_RAVE_MODE_ADRESS              (51U)
#define EEPROM_RAINBOW_STEP_ADRESS           (52U)
#define EEPROM_MAX_COEF_FREQ_ADRESS          (56U)
#define EEPROM_STROBE_PERIOD_ADRESS          (60U)
#define EEPROM_RAINBOW_STEP_2_ADRESS         (64U)
#define EEPROM_SMOOTH_ADRESS                 (70U)
#define EEPROM_SMOOTH_FREQ_ADRESS            (74U)
#define EEPROM_STROBE_SMOOTH_ADRESS          (78U)
#define EEPROM_COLOR_SPEED_ADRESS            (82U)
#define EEPROM_RAINBOW_PERIOD_ADRESS         (86U)
#define EEPROM_RUNNING_SPEED_ADRESS          (90U)
#define EEPROM_LAMP_ID_ADRESS                (98U)
#define EEPROM_LOW_PASS_ADRESS               (100U)
#define EEPROM_SPEKTR_LOW_PASS_ADRESS        (102U)

#define EEPROM_MODE_STRUCT_SIZE              (5U)           // 1 байт - яркость; 1 байт - яркость фона; 1 байт - скорость, 1 байт - hue, 1 байт - насыщенность 

#define EEPROM_FIRST_RUN_MARK                (24U)          // число-метка, если ещё не записно в EEPROM_FIRST_RUN_ADDRESS, значит нужно проинициализировать EEPROM и записать все первоначальные настройки
#define EEPROM_WRITE_DELAY                   (30000UL)      // отсрочка записи в EEPROM после последнего изменения хранимых настроек, позволяет уменьшить количество операций записи в EEPROM

void initEEPROM() {
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

        EEPROM.write(EEPROM_ESP_MODE, ESP_MODE);
        EEPROM.write(EEPROM_LAMP_ON_ADDRESS, 0);
        EEPROM.write(EEPROM_CURRENT_MODE_ADDRESS, 0);
        EEPROM.write(EEPROM_RAVE_MODE_ADRESS, 0);
        EEPROM.write(EEPROM_RAINBOW_STEP_ADRESS, RAINBOW_STEP);
        EEPROM.write(EEPROM_MAX_COEF_FREQ_ADRESS, MAX_COEF_FREQ);
        EEPROM.write(EEPROM_STROBE_PERIOD_ADRESS, STROBE_PERIOD);
        EEPROM.write(EEPROM_RAINBOW_STEP_2_ADRESS, RAINBOW_STEP_2);
        EEPROM.write(EEPROM_SMOOTH_ADRESS, SMOOTH);
        EEPROM.write(EEPROM_SMOOTH_FREQ_ADRESS, SMOOTH_FREQ);
        EEPROM.write(EEPROM_STROBE_SMOOTH_ADRESS, STROBE_SMOOTH);
        EEPROM.write(EEPROM_COLOR_SPEED_ADRESS, COLOR_SPEED);
        EEPROM.write(EEPROM_RAINBOW_PERIOD_ADRESS, RAINBOW_PERIOD);
        EEPROM.write(EEPROM_RUNNING_SPEED_ADRESS, RUNNING_SPEED);
        EEPROM.write(EEPROM_LAMP_ID_ADRESS, LAMP_ID);

        ONstate = (bool)EEPROM.read(EEPROM_LAMP_ON_ADDRESS);
        this_mode = EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS);

       
    
        EEPROM.commit();
      }

}
void SaveModesSettings(int8_t this_mode)
    {
      EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * ((int)this_mode), modes[(int)this_mode]);

      EEPROM.commit();
    }

void updateEEPROM() {
  SaveModesSettings((int)this_mode);
  EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * (this_mode), modes[this_mode]);
  EEPROM.write(EEPROM_ESP_MODE, ESP_MODE);
  EEPROM.write(EEPROM_CURRENT_MODE_ADDRESS, this_mode);
  EEPROM.write(EEPROM_RAVE_MODE_ADRESS, RAVE_MODE);
  EEPROM.write(EEPROM_RAINBOW_STEP_ADRESS, RAINBOW_STEP);
  EEPROM.write(EEPROM_MAX_COEF_FREQ_ADRESS, MAX_COEF_FREQ);
  EEPROM.write(EEPROM_STROBE_PERIOD_ADRESS, STROBE_PERIOD);
  EEPROM.write(EEPROM_RAINBOW_STEP_2_ADRESS, RAINBOW_STEP_2);
  EEPROM.write(EEPROM_SMOOTH_ADRESS, SMOOTH);
  EEPROM.write(EEPROM_SMOOTH_FREQ_ADRESS, SMOOTH_FREQ);
  EEPROM.write(EEPROM_STROBE_SMOOTH_ADRESS, STROBE_SMOOTH);
  EEPROM.write(EEPROM_COLOR_SPEED_ADRESS, COLOR_SPEED);
  EEPROM.write(EEPROM_RAINBOW_PERIOD_ADRESS, RAINBOW_PERIOD);
  EEPROM.write(EEPROM_RUNNING_SPEED_ADRESS, RUNNING_SPEED);
  EEPROM.write(EEPROM_LAMP_ID_ADRESS, LAMP_ID);

  if (KEEP_STATE) EEPROM.write(EEPROM_LAMP_ON_ADDRESS, (int)ONstate);

  EEPROM.commit();
}
void readEEPROM() {
  this_mode = EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS);
  RAVE_MODE = EEPROM.read(EEPROM_RAVE_MODE_ADRESS);
  RAINBOW_STEP = EEPROM.read(EEPROM_RAINBOW_STEP_ADRESS);
  MAX_COEF_FREQ = EEPROM.read(EEPROM_MAX_COEF_FREQ_ADRESS);
  STROBE_PERIOD = EEPROM.read(EEPROM_STROBE_PERIOD_ADRESS);
  RAINBOW_STEP_2 = EEPROM.read(EEPROM_RAINBOW_STEP_2_ADRESS);
  SMOOTH = EEPROM.read(EEPROM_SMOOTH_ADRESS);
  SMOOTH_FREQ = EEPROM.read(EEPROM_SMOOTH_FREQ_ADRESS);
  STROBE_SMOOTH = EEPROM.read(EEPROM_STROBE_SMOOTH_ADRESS);
  LIGHT_COLOR = EEPROM.read(EEPROM_COLOR_SPEED_ADRESS);
  COLOR_SPEED = EEPROM.read(EEPROM_RAINBOW_PERIOD_ADRESS);
  RAINBOW_PERIOD = EEPROM.read(EEPROM_RUNNING_SPEED_ADRESS);
  RUNNING_SPEED = EEPROM.read(EEPROM_LAMP_ID_ADRESS);
  if (KEEP_STATE) ONstate = EEPROM.read(EEPROM_LAMP_ON_ADDRESS);

  EEPROM.commit();
}
void eepromTick() {
  if (eeprom_flag)
    if (millis() - eeprom_timer > EEPROM_WRITE_DELAY) {  // 30 секунд после последнего нажатия с пульта
      eeprom_flag = false;
      eeprom_timer = millis();
      updateEEPROM();
    }
}
