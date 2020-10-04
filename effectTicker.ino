void animation() {
  // согласно режиму
  switch (this_mode) {
    case 0: //подсветка
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LIGHT_COLOR, LIGHT_SAT, BRIGHTNESS);
          break;   
    case 1: //подсветка
      if (millis() - color_timer > COLOR_SPEED) {
            color_timer = millis();
            if (++this_color > 255) this_color = 0;
          }
          for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, LIGHT_SAT, BRIGHTNESS);
          break;
    case 2: //подсветка
      if (millis() - rainbow_timer > 30) {
            rainbow_timer = millis();
            this_color += RAINBOW_PERIOD;
            if (this_color > 255) this_color = 0;
            if (this_color < 0) this_color = 255;
          }
          rainbow_steps = this_color;
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV((int)floor(rainbow_steps), 255, BRIGHTNESS);
            rainbow_steps += RAINBOW_STEP_2;
            if (rainbow_steps > 255) rainbow_steps = 0;
            if (rainbow_steps < 0) rainbow_steps = 255;
          }
          break;
    case 3: //зеленая-красная громкость
       count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(myPal, (count * index), BRIGHTNESS);   // заливка по палитре " от зелёного к красному"
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(myPal, (count * index), BRIGHTNESS);   // заливка по палитре " от зелёного к красному"
        count++;
      }
      if (EMPTY_BRIGHT > 0) {
        CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
          leds[i] = this_dark;
        for (int i = MAX_CH + Llenght; i < NUM_LEDS; i++)
          leds[i] = this_dark;
      }
      break;
    case 4: //радуга-громкость
      if (millis() - rainbow_timer > 30) {
        rainbow_timer = millis();
        hue = floor((float)hue + RAINBOW_STEP);
      }
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue, BRIGHTNESS);  // заливка по палитре радуга
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Llenght); i++ ) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * index) / 2 - hue, BRIGHTNESS); // заливка по палитре радуга
        count++;
      }
      if (EMPTY_BRIGHT > 0) {
        CHSV this_dark = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
        for (int i = ((MAX_CH - 1) - Rlenght); i > 0; i--)
          leds[i] = this_dark;
        for (int i = MAX_CH + Llenght; i < NUM_LEDS; i++)
          leds[i] = this_dark;
      }
      break;
    case 5: //светомузыка
         for (int i = 0; i < NUM_LEDS; i++) {
          if (i < STRIPE)          leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
          else if (i < STRIPE * 2) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
          else if (i < STRIPE * 3) leds[i] = CHSV(LOW_COLOR, 255, thisBright[0]);
          else if (i < STRIPE * 4) leds[i] = CHSV(MID_COLOR, 255, thisBright[1]);
          else if (i < STRIPE * 5) leds[i] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        }
        break;
    case 6: //бегущие частоты
        if (running_flag[2]) leds[NUM_LEDS / 2] = CHSV(HIGH_COLOR, 255, thisBright[2]);
        else if (running_flag[1]) leds[NUM_LEDS / 2] = CHSV(MID_COLOR, 255, thisBright[1]);
        else if (running_flag[0]) leds[NUM_LEDS / 2] = CHSV(LOW_COLOR, 255, thisBright[0]);
        else leds[NUM_LEDS / 2] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
      leds[(NUM_LEDS / 2) - 1] = leds[NUM_LEDS / 2];
      if (millis() - running_timer > RUNNING_SPEED) {
        running_timer = millis();
        for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {
          leds[i] = leds[i + 1];
          leds[NUM_LEDS - i - 1] = leds[i];
        }
      }
      break;
    case 7: //стробоскоп-реакция
      if (colorMusicFlash[0]) for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(LIGHT_COLOR, LIGHT_SAT, thisBright[0]);
      break;
    case 8: //огонь
      fireRoutine(false);
      delay(40);
      generateValues = true;
      break;
  }
}

void LOWS() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(STROBE_COLOR, STROBE_SAT, thisBright[0]);
}
void SILENCE() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(EMPTY_COLOR, 255, EMPTY_BRIGHT);
}
