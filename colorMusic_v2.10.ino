



// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------
#define MODE_AMOUNT 9      // количество режимов

#define STRIPE NUM_LEDS / 5
float freq_to_stripe = NUM_LEDS / 40; // /2 так как симметрия, и /20 так как 20 частот

#define FHT_N 64         // ширина спектра х2
#define LOG_OUT 1
#include <FHT.h>         // преобразование Хартли

#include <EEPROMex.h>

#define FASTLED_ALLOW_INTERRUPTS 1
#include "FastLED.h"
CRGB leds[NUM_LEDS];

#include "GyverButton.h"
GButton butt1(BTN_PIN);

#include "IRLremote.h"
CHashIR IRLremote;
uint32_t IRdata;

// градиент-палитра от зелёного к красному
DEFINE_GRADIENT_PALETTE(soundlevel_gp) {
  0,    0,    255,  0,  // green
  100,  255,  255,  0,  // yellow
  150,  255,  100,  0,  // orange
  200,  255,  50,   0,  // red
  255,  255,  0,    0   // red
};
CRGBPalette32 myPal = soundlevel_gp;

int Rlenght, Llenght;
float RsoundLevel, RsoundLevel_f;
float LsoundLevel, LsoundLevel_f;

float averageLevel = 50;
int maxLevel = 100;
int MAX_CH = NUM_LEDS / 2;
int hue;
unsigned long main_timer, hue_timer, strobe_timer, running_timer, color_timer, rainbow_timer, eeprom_timer;
float averK = 0.006;
byte count;
float index = (float)255 / MAX_CH;   // коэффициент перевода для палитры
boolean lowFlag;
byte low_pass;
int RcurrentLevel, LcurrentLevel;
int colorMusic[3];
float colorMusic_f[3], colorMusic_aver[3];
boolean colorMusicFlash[3], strobeUp_flag, strobeDwn_flag;
byte this_mode = MODE;
int thisBright[3], strobe_bright = 0;
unsigned int light_time = STROBE_PERIOD * STROBE_DUTY / 100;
volatile boolean ir_flag;
boolean settings_mode, ONstate = true;
int freq_max;
float freq_max_f, rainbow_steps;
int freq_f[32];
int this_color;
boolean running_flag[3], eeprom_flag;

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// ------------------------------ ДЛЯ РАЗРАБОТЧИКОВ --------------------------------

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(BRIGHTNESS);

#if defined(__AVR_ATmega32U4__)   //Выключение светодиодов на Pro Micro
  TXLED1;                           //на ProMicro выключим и TXLED
  delay (1000);                     //При питании по usb от компьютера нужна задержка перед выключением RXLED. Если питать от БП, то можно убрать эту строку.
#endif
  pinMode(MLED_PIN, OUTPUT);        //Режим пина для светодиода режима на выход
  digitalWrite(MLED_PIN, !MLED_ON); //Выключение светодиода режима

  pinMode(POT_GND, OUTPUT);
  digitalWrite(POT_GND, LOW);
  butt1.setTimeout(900);

  IRLremote.begin(IR_PIN);

  // для увеличения точности уменьшаем опорное напряжение,
  // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
  // GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3
  // в данной схеме GND берётся из А0 для удобства подключения
  if (POTENT) analogReference(EXTERNAL);
  else
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
#else
    analogReference(INTERNAL);
#endif

  // жуткая магия, меняем частоту оцифровки до 18 кГц
  // команды на ебучем ассемблере, даже не спрашивайте, как это работает
  // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
  // Котельникова (Найквиста) частота дискретизации будет 19.2 кГц
  // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  if (RESET_SETTINGS) EEPROM.write(100, 0);        // сброс флага настроек

  if (AUTO_LOW_PASS && !EEPROM_LOW_PASS) {         // если разрешена автонастройка нижнего порога шумов
    autoLowPass();
  }
  if (EEPROM_LOW_PASS) {                // восстановить значения шумов из памяти
    LOW_PASS = EEPROM.readInt(70);
    SPEKTR_LOW_PASS = EEPROM.readInt(72);
  }

  // в 100 ячейке хранится число 100. Если нет - значит это первый запуск системы
  if (KEEP_SETTINGS) {
    if (EEPROM.read(100) != 100) {
      //Serial.println(F("First start"));
      EEPROM.write(100, 100);
      updateEEPROM();
    } else {
      readEEPROM();
    }
  }

#if (SETTINGS_LOG == 1)
  Serial.print(F("this_mode = ")); Serial.println(this_mode);
  Serial.print(F("RAVE_MODE = ")); Serial.println(RAVE_MODE);
  Serial.print(F("RAINBOW_STEP = ")); Serial.println(RAINBOW_STEP);
  Serial.print(F("MAX_COEF_FREQ = ")); Serial.println(MAX_COEF_FREQ);
  Serial.print(F("STROBE_PERIOD = ")); Serial.println(STROBE_PERIOD);
  Serial.print(F("LIGHT_SAT = ")); Serial.println(LIGHT_SAT);
  Serial.print(F("RAINBOW_STEP_2 = ")); Serial.println(RAINBOW_STEP_2);
  Serial.print(F("SMOOTH = ")); Serial.println(SMOOTH);
  Serial.print(F("SMOOTH_FREQ = ")); Serial.println(SMOOTH_FREQ);
  Serial.print(F("STROBE_SMOOTH = ")); Serial.println(STROBE_SMOOTH);
  Serial.print(F("LIGHT_COLOR = ")); Serial.println(LIGHT_COLOR);
  Serial.print(F("COLOR_SPEED = ")); Serial.println(COLOR_SPEED);
  Serial.print(F("RAINBOW_PERIOD = ")); Serial.println(RAINBOW_PERIOD);
  Serial.print(F("RUNNING_SPEED = ")); Serial.println(RUNNING_SPEED);
  Serial.print(F("EMPTY_BRIGHT = ")); Serial.println(EMPTY_BRIGHT);
  Serial.print(F("ONstate = ")); Serial.println(ONstate);
#endif
}

void loop() {
  buttonTick();     // опрос и обработка кнопки
  remoteTick();     // опрос ИК пульта
  mainLoop();       // главный цикл обработки и отрисовки
  eepromTick();     // проверка не пора ли сохранить настройки
}

void mainLoop() {
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
          if (!MONO) LcurrentLevel = analogRead(SOUND_L);                 // и левого каналов

          if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
          if (!MONO) if (LsoundLevel < LcurrentLevel) LsoundLevel = LcurrentLevel;   // ищем максимальное
        }

        // фильтруем по нижнему порогу шумов
        RsoundLevel = map(RsoundLevel, LOW_PASS, 1023, 0, 500);
        if (!MONO)LsoundLevel = map(LsoundLevel, LOW_PASS, 1023, 0, 500);

        // ограничиваем диапазон
        RsoundLevel = constrain(RsoundLevel, 0, 500);
        if (!MONO)LsoundLevel = constrain(LsoundLevel, 0, 500);

        // возводим в степень (для большей чёткости работы)
        RsoundLevel = pow(RsoundLevel, EXP);
        if (!MONO)LsoundLevel = pow(LsoundLevel, EXP);

        // фильтр
        RsoundLevel_f = RsoundLevel * SMOOTH + RsoundLevel_f * (1 - SMOOTH);
        if (!MONO)LsoundLevel_f = LsoundLevel * SMOOTH + LsoundLevel_f * (1 - SMOOTH);

        if (MONO) LsoundLevel_f = RsoundLevel_f;  // если моно, то левый = правому

        // заливаем "подложку", если яркость достаточная
        if (EMPTY_BRIGHT > 5) {
          for (int i = 0; i < NUM_LEDS; i++)
            leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
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
      if (this_mode == 5 || this_mode == 6 || this_mode == 7) {
        analyzeAudio();
        colorMusic[0] = 0;
        colorMusic[1] = 0;
        colorMusic[2] = 0;
        for (int i = 0 ; i < 32 ; i++) {
          if (fht_log_out[i] < SPEKTR_LOW_PASS) fht_log_out[i] = 0;
        }
        // низкие частоты, выборка со 2 по 5 тон (0 и 1 зашумленные!)
        for (byte i = 2; i < 4; i++) {
          if (fht_log_out[i] > colorMusic[0]) colorMusic[0] = fht_log_out[i];
        }
        // средние частоты, выборка с 6 по 10 тон
        for (byte i = 6; i < 11; i++) {
          if (fht_log_out[i] > colorMusic[1]) colorMusic[1] = fht_log_out[i];
        }
        // высокие частоты, выборка с 11 по 31 тон
        for (byte i = 11; i < 32; i++) {
          if (fht_log_out[i] > colorMusic[2]) colorMusic[2] = fht_log_out[i];
        }
        freq_max = 0;
        for (byte i = 0; i < 30; i++) {
          if (fht_log_out[i + 2] > freq_max) freq_max = fht_log_out[i + 2];
          if (freq_max < 5) freq_max = 5;

          if (freq_f[i] < fht_log_out[i + 2]) freq_f[i] = fht_log_out[i + 2];
          else freq_f[i] = 0;
        }
        freq_max_f = freq_max * averK + freq_max_f * (1 - averK);
        for (byte i = 0; i < 3; i++) {
          colorMusic_aver[i] = colorMusic[i] * averK + colorMusic_aver[i] * (1 - averK);  // общая фильтрация
          colorMusic_f[i] = colorMusic[i] * SMOOTH_FREQ + colorMusic_f[i] * (1 - SMOOTH_FREQ);      // локальная
          if (colorMusic_f[i] > ((float)colorMusic_aver[i] * MAX_COEF_FREQ)) {
            thisBright[i] = BRIGHTNESS;
            colorMusicFlash[i] = true;
            running_flag[i] = true;
          } else colorMusicFlash[i] = false;
          if (thisBright[i] >= 0) thisBright[i] -= SMOOTH_STEP;
          if (thisBright[i] < EMPTY_BRIGHT) {
            thisBright[i] = EMPTY_BRIGHT;
            running_flag[i] = false;
          }
        }
        animation();
      }
      if (this_mode == 0 || this_mode == 1 ||this_mode == 2 || this_mode == 8) animation();

      if (!IRLremote.receiving())    // если на ИК приёмник не приходит сигнал (без этого НЕ РАБОТАЕТ!)
        FastLED.show();         // отправить значения на ленту

      if (this_mode != 6 && this_mode != 8)       // 6 режиму не нужна очистка!!!
        FastLED.clear();          // очистить массив пикселей
      main_timer = millis();    // сбросить таймер
    }
  }
}





void remoteTick() {
  if (IRLremote.available())  {
    auto data = IRLremote.read();
    IRdata = data.command;
    ir_flag = true;
  }
  if (ir_flag) { // если данные пришли
    eeprom_timer = millis();
    eeprom_flag = true;
    switch (IRdata) {
      // режимы
      case BUTT_1: this_mode = 0;
        break;
      case BUTT_2: this_mode = 1;
        break;
      case BUTT_3: this_mode = 2;
        break;
      case BUTT_4: this_mode = 3;
        break;
      case BUTT_5: this_mode = 4;
        break;
      case BUTT_6: this_mode = 5;
        BRIGHTNESS = BRIGHTNESS < 50 ? 50 : BRIGHTNESS;
        MAX_COEF_FREQ = 1.2;
        break;
      case BUTT_7: this_mode = 6;
        BRIGHTNESS = BRIGHTNESS < 50 ? 50 : BRIGHTNESS;
        MAX_COEF_FREQ = 0.7;
        break;
      case BUTT_8: this_mode = 7;
        BRIGHTNESS = BRIGHTNESS < 50 ? 50 : BRIGHTNESS;
        break;
      case BUTT_9: this_mode = 8;
        break;
      case BUTT_0: fullLowPass();
        break;
      case BUTT_STAR: ONstate = !ONstate; FastLED.clear(); FastLED.show(); updateEEPROM();
        break;
      case BUTT_HASH:
        RAVE_MODE = !RAVE_MODE;
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
        break;
      case BUTT_OK: digitalWrite(MLED_PIN, settings_mode ^ MLED_ON); settings_mode = !settings_mode;
        break;
      case BUTT_UP:
      Serial.println(BRIGHTNESS);
        if (settings_mode) {
          // ВВЕРХ общие настройки
          BRIGHTNESS = smartIncr(BRIGHTNESS, 30, 20, 255);
        } else {
          switch (this_mode) {
            case 0: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 20, 255);
              break;
            case 1: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
              break;
            case 7: LIGHT_SAT = smartIncr(LIGHT_SAT, 20, 0, 255);
              break;
            default: BRIGHTNESS = smartIncr(BRIGHTNESS, 30, 20, 255);
              break;
          }
        }
        break;
      case BUTT_DOWN:
      Serial.println(BRIGHTNESS);
        if (settings_mode) {
          // ВНИЗ общие настройки
          BRIGHTNESS = smartIncr(BRIGHTNESS, -30, 20, 255);
        } else {
          switch (this_mode) {
            case 0: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
              break;
            case 1: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
              break;
            case 7: LIGHT_SAT = smartIncr(LIGHT_SAT, -20, 0, 255);
              break;
            default: BRIGHTNESS = smartIncr(BRIGHTNESS, -30, 20, 255);
              break;
          }
        }
        break;
      case BUTT_LEFT:
        if (settings_mode) {
          // ВЛЕВО общие настройки
          EMPTY_BRIGHT = smartIncr(EMPTY_BRIGHT, -50, 0, 255);
          FastLED.setBrightness(BRIGHTNESS);
        } else {
          switch (this_mode) {
            case 0: LIGHT_COLOR = smartIncr(LIGHT_COLOR, -10, 0, 255);
              break;
            case 1: LIGHT_COLOR = smartIncr(LIGHT_COLOR, -10, 0, 255);
              break;
            case 2: RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, -1, 1, 10);
               RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, -2, 2, 20);
              break;
            case 3: SMOOTH = smartIncrFloat(SMOOTH, -0.20, 0.05, 1);
              break;
            case 4: SMOOTH = smartIncrFloat(SMOOTH, -0.20, 0.05, 1);
              RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, -4, 3.5, 20);
              break;
            case 6: RUNNING_SPEED = smartIncr(RUNNING_SPEED, 20, 1, 60);
              break;
            case 7: LIGHT_COLOR = smartIncr(LIGHT_COLOR, -10, 0, 255);
              break;
          }
        }
        break;
      case BUTT_RIGHT:
        if (settings_mode) {
          // ВПРАВО общие настройки
          EMPTY_BRIGHT = smartIncr(EMPTY_BRIGHT, 50, 0, 255);
          FastLED.setBrightness(BRIGHTNESS);
        } else {
          switch (this_mode) {
            case 0: LIGHT_COLOR = smartIncr(LIGHT_COLOR, 10, 0, 255);
              break;
            case 2: RAINBOW_STEP_2 = smartIncrFloat(RAINBOW_STEP_2, 1, 1, 10);
              RAINBOW_PERIOD = smartIncr(RAINBOW_PERIOD, 2, 2, 20);
              break;
            case 3: SMOOTH = smartIncrFloat(SMOOTH, 0.20, 0.05, 1);
              break;
            case 4: SMOOTH = smartIncrFloat(SMOOTH, 0.20, 0.05, 1);
              RAINBOW_STEP = smartIncrFloat(RAINBOW_STEP, 4, 3.5, 20);
              break;
            case 6: RUNNING_SPEED = smartIncr(RUNNING_SPEED, -20, 1, 60);
              break;
            case 7: LIGHT_COLOR = smartIncr(LIGHT_COLOR, 10, 0, 255);
              break;
          }
        }
        break;
      default: eeprom_flag = false;   // если не распознали кнопку, не обновляем настройки!
        break;
    }
    ir_flag = false;
  }
}



void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(SOUND_R_FREQ);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
}
