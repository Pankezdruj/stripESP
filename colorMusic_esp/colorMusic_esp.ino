
#include <ESP8266WebServer.h>
#include "pgmspace.h"
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "Constants.h"
#include "CaptivePortalManager.h"
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "EepromManager.h"

// ----- пины подключения
#define SOUND_R A0         // аналоговый пин вход аудио, правый канал
//#define SOUND_L A1         // аналоговый пин вход аудио, левый канал
#define SOUND_R_FREQ A0    // аналоговый пин вход аудио для режима с частотами (через кондер)
#define BTN_PIN 3          // кнопка переключения режимов (PIN --- КНОПКА --- GND)
                         
#define MLED_PIN 13             // пин светодиода режимов
#define MLED_ON HIGH
#define LED_PIN 12              // пин DI светодиодной ленты

void mainLoop() {
  if(RAVE_MODE) {
          SMOOTH_FREQ = 1.0;
          RUNNING_SPEED = 1;
          RAINBOW_PERIOD = 10.00;
          RAINBOW_STEP_2 = 2;
          SMOOTH = 1.0;
          RAINBOW_STEP = 20.00;
        } else {
          SMOOTH_FREQ = 0.3;
          RUNNING_SPEED = 51;
          RAINBOW_PERIOD = 10.00;
          RAINBOW_STEP_2 = 2;
          SMOOTH = 0.05;
          RAINBOW_STEP = 3.50;
        }
  // главный цикл отрисовки
  if (ONstate) {
    if (millis() - main_timer > MAIN_LOOP) {
      // сбрасываем значения
      RsoundLevel = 0;
      LsoundLevel = 0;

      // перваые два режима - громкость (VU meter)
      if (this_mode == 3 || this_mode == 4) {
        for (byte i = 0; i < 100; i ++) {                                 // делаем 100 измерений
          RcurrentLevel = analogRead(SOUND_R);                            // с правого
          //if (!MONO) LcurrentLevel = analogRead(SOUND_L);                 // и левого каналов

          if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
          //if (!MONO) if (LsoundLevel < LcurrentLevel) LsoundLevel = LcurrentLevel;   // ищем максимальное
        }

        // фильтруем по нижнему порогу шумов
        RsoundLevel = map(RsoundLevel, LOW_PASS, 1023, 0, 500);
        //if (!MONO)LsoundLevel = map(LsoundLevel, LOW_PASS, 1023, 0, 500);

        // ограничиваем диапазон
        RsoundLevel = constrain(RsoundLevel, 0, 500);
        //if (!MONO)LsoundLevel = constrain(LsoundLevel, 0, 500);

        // возводим в степень (для большей чёткости работы)
        RsoundLevel = pow(RsoundLevel, EXP);
        //if (!MONO)LsoundLevel = pow(LsoundLevel, EXP);

        // фильтр
        RsoundLevel_f = RsoundLevel * SMOOTH + RsoundLevel_f * (1 - SMOOTH);
        //if (!MONO)LsoundLevel_f = LsoundLevel * SMOOTH + LsoundLevel_f * (1 - SMOOTH);

        //if (MONO) LsoundLevel_f = RsoundLevel_f;  // если моно, то левый = правому

        // заливаем "подложку", если яркость достаточная
        if (EMPTY_BRIGHT > 5) {
          for (int i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(EMPTY_COLOR, 255, modes[this_mode].BGBrightness);
        }

        // если значение выше порога - начинаем самое интересное
        if (RsoundLevel_f > 15 && LsoundLevel_f > 15) {

          // расчёт общей средней громкости с обоих каналов, фильтрация.
          // Фильтр очень медленный, сделано специально для автогромкости
          averageLevel = (float)(RsoundLevel_f + LsoundLevel_f) / 2 * averK + averageLevel * (1 - averK);

          // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
          maxLevel = (float)averageLevel * MAX_COEF;

          // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
          Rlenght = map(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);
          Llenght = map(LsoundLevel_f, 0, maxLevel, 0, MAX_CH);

          // ограничиваем до макс. числа светодиодов
          Rlenght = constrain(Rlenght, 0, MAX_CH);
          Llenght = constrain(Llenght, 0, MAX_CH);

          animation();       // отрисовать
        }
      }

      // 5,6,7 режим - цветомузыка
//      if (this_mode == 5 || this_mode == 6 || this_mode == 7) {
//        analyzeAudio();
//        colorMusic[0] = 0;
//        colorMusic[1] = 0;
//        colorMusic[2] = 0;
//        for (int i = 0 ; i < 32 ; i++) {
//          if (fht_log_out[i] < SPEKTR_LOW_PASS) fht_log_out[i] = 0;
//        }
//        // низкие частоты, выборка со 2 по 5 тон (0 и 1 зашумленные!)
//        for (byte i = 2; i < 4; i++) {
//          if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
//        }
//        // средние частоты, выборка с 6 по 10 тон
//        for (byte i = 6; i < 11; i++) {
//          if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
//        }
//        // высокие частоты, выборка с 11 по 31 тон
//        for (byte i = 11; i < 32; i++) {
//          if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
//        }
//        freq_max = 0;
//        for (byte i = 0; i < 30; i++) {
//          if (fht_log_out[i + 2] > freq_max) freq_max = fht_log_out[i + 2];
//          if (freq_max < 5) freq_max = 5;
//
//          if (freq_f[i] < fht_log_out[i + 2]) freq_f[i] = fht_log_out[i + 2];
//          else freq_f[i] = 0;
//        }
//        freq_max_f = freq_max * averK + freq_max_f * (1 - averK);
//        for (byte i = 0; i < 3; i++) {
//          colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);  // общая фильтрация
//          colorMusic_f[i] = colorMusic[i] * SMOOTH_FREQ + colorMusic_f[i] * (1 - SMOOTH_FREQ);      // локальная
//          if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
//            thisBright[i] = modes[this_mode].Brightness;
//            colorMusicFlash[i] = true;
//            running_flag[i] = true;
//          } else colorMusicFlash[i] = false;
//          if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
//          if (thisBright[i] < EMPTY_BRIGHT) {
//            thisBright[i] = EMPTY_BRIGHT;
//            running_flag[i] = false;
//          }
//        }
//        animation();
//      }
      if (this_mode == 0 || this_mode == 1 ||this_mode == 2 || this_mode == 8) animation();

        FastLED.show();         // отправить значения на ленту

      if (this_mode != 6 && this_mode != 8)       // 6 режиму не нужна очистка!!!
        FastLED.clear();          // очистить массив пикселей
      main_timer = millis();    // сбросить таймер
    }
  }
}





void settingsTick() {
  switch(this_mode) {
    case(2):
        RAINBOW_STEP_2 = map(modes[2].Speed, 0, 255, 1, 10);
        RAINBOW_PERIOD = map(modes[2].Speed, 0, 255, 2, 20);
        break;
    case(3):
        SMOOTH = map(modes[3].Speed, 0, 255, 0.05, 1);
        break;
    case(4):
        SMOOTH = map(modes[4].Speed, 0, 255, 0.05, 1);
        RAINBOW_STEP_2 = map(modes[4].Speed, 0, 255, 3.5, 20);
        break;
    case(5):
        modes[5].Brightness = modes[5].Brightness < 50 ? 50 : modes[5].Brightness;
        MAX_COEF_FREQ = 1.2;
        break;
    case(6):
        modes[6].Brightness = modes[6].Brightness < 50 ? 50 : modes[6].Brightness;
        RUNNING_SPEED = map(modes[6].Speed, 0, 255, 1, 60);
        MAX_COEF_FREQ = 0.7;
        break;
    case(7):
        modes[7].Brightness = modes[7].Brightness < 50 ? 50 : modes[7].Brightness;
        break;
  }
  FastLED.setBrightness(modes[this_mode].Brightness);
}



void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(SOUND_R_FREQ);
   // fht_input[i] = sample; // put real data into bins
  }
//  fht_window();  // window the data for better frequency response
//  fht_reorder(); // reorder the data before doing the fht
//  fht_run();     // process the data in the fht
//  fht_mag_log(); // take the output of the fht
}
