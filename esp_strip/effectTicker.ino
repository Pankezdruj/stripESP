uint32_t effTimer;

void effectsTick()
{
    if (ONflag && (millis() - effTimer >= MAIN_LOOP))
    {
      effTimer = millis();
       // сбрасываем значения
      RsoundLevel = 0;
      LsoundLevel = 0;
      maxLevel = 0;

        for (byte i = 0; i < 100; i ++) {                                 // делаем 100 измерений
          RcurrentLevel = analogRead(SOUND_R);
          if (RsoundLevel < RcurrentLevel) RsoundLevel = RcurrentLevel;   // ищем максимальное
        }

        // фильтруем по нижнему порогу шумов
        RsoundLevel = secureMap(RsoundLevel, LOW_PASS, 1023, 0, 500);

        // ограничиваем диапазон
        //RsoundLevel = constrain(RsoundLevel, 0, 500);

        // возводим в степень (для большей чёткости работы)
        //RsoundLevel = pow(RsoundLevel, EXP);
        //Serial.print(RcurrentLevel);
        //Serial.print(",");
        // фильтр
        if(RsoundLevel_f < 1) RsoundLevel_f = 1; else if (RsoundLevel_f > 1000) RsoundLevel_f = 200;
        if (currentMode == 3 || currentMode == 9) RsoundLevel_f = (RsoundLevel * SMOOTH) + (RsoundLevel_f * (1 - SMOOTH));
        else RsoundLevel_f = RsoundLevel; 
        //Serial.print(RsoundLevel_f);
        //Serial.print(",");
        
        averageLevel = (float)RsoundLevel_f * averK + averageLevel * (1 - averK);
        if(averageLevel < 2) averageLevel = 2;
        //Serial.print(averageLevel);
        //Serial.print(",");
        // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
        maxLevel = (float)averageLevel * MAX_COEF;
        if(maxLevel < 5) maxLevel = 5;
        if(maxLevel < RsoundLevel_f) maxLevel = RsoundLevel_f;
        //Serial.print(maxLevel);
        
        thisBright[0] = RsoundLevel_f;
        thisBright[1] = averageLevel;
        thisBright[2] = maxLevel;
        
        if(RsoundLevel_f > averageLevel+20) colorMusicFlash = true;
        else colorMusicFlash = false;
        
        if (thisBright[3] > 0) thisBright[3] -= SMOOTH_STEP; else if(colorMusicFlash) thisBright[3] = 255;
        if (thisBright[3] < 0) thisBright[3] = 0;

        // преобразуем сигнал в длину ленты (где MAX_CH это половина количества светодиодов)
        
        Rlenght = secureMap(RsoundLevel_f, 0, maxLevel, 0, MAX_CH);
        animation();  
      FastLED.show();
    }
}

void changePower()
{
  if (ONflag)
  {
    effectsTick();
    for (uint8_t i = 0U; i < modes[currentMode].Brightness; i = constrain(i + 8, 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.setBrightness(modes[currentMode].Brightness);
    delay(2);
    FastLED.show();
  }
  else
  {
    effectsTick();
    for (uint8_t i = modes[currentMode].Brightness; i > 0; i = constrain(i - 8, 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.clear();
    delay(2);
    FastLED.show();
  }
}
void animation() {
  int scrollSpeed = 0;
  // согласно режиму
  switch (currentMode) {
    case 0: //подсветка
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(modes[currentMode].Color[0], modes[currentMode].Color[1], modes[currentMode].Brightness);
          break;   
    case 1: //подсветка
      if (millis() - color_timer > COLOR_SPEED) {
            color_timer = millis();
            this_color += COLOR_SPEED > 100 ? 1 : map(COLOR_SPEED, 100, 0, 1, 10); 
            if (this_color > 255) this_color = 0;
          }
          for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, modes[currentMode].Color[1], modes[currentMode].Brightness);
          break;
    case 2: //радуга
      if (millis() - rainbow_timer > 30) {
            rainbow_timer = millis();
            this_color += RAINBOW_PERIOD;
            if (this_color > 255) this_color = 0;
            if (this_color < 0) this_color = 255;
          }
          rainbow_steps = this_color;
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV((int)floor(rainbow_steps), 255, modes[currentMode].Brightness);
            rainbow_steps += RAINBOW_STEP_2;
            if (rainbow_steps > 255) rainbow_steps = 0;
            if (rainbow_steps < 0) rainbow_steps = 255;
          }
          break;
    case 3: //радуга-громкость
      if (millis() - rainbow_timer > 30) {
        rainbow_timer = millis();
        hue = floor((float)hue + RAINBOW_STEP); 
      }
      count = 0;
      for (int i = (MAX_CH - 1); i >= 0; i--) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * ind) / 2 - hue, modes[currentMode].BGBrightness);  // заливка по палитре радуга
        count++;
      }
      count = 0;
      for (int i = MAX_CH; i <= NUM_LEDS; i++ ) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * ind) / 2 - hue, modes[currentMode].BGBrightness); // заливка по палитре радуга
        count++;
      } 
      count = 0;
      for (int i = (MAX_CH - 1); i > ((MAX_CH - 1) - Rlenght); i--) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * ind) / 2 - hue, modes[currentMode].Brightness);  // заливка по палитре радуга
        count++;
      }
      count = 0;
      for (int i = (MAX_CH); i < (MAX_CH + Rlenght); i++ ) {
        leds[i] = ColorFromPalette(RainbowColors_p, (count * ind) / 2 - hue, modes[currentMode].Brightness); // заливка по палитре радуга
        count++;
      }
      break;
    case 4: //конфетти
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(random(255), 255, modes[currentMode].Brightness);
        delay(60);
        break;
    case 5: //широкое конфетти 
          
        for (int i = 0; i < NUM_LEDS; i++) {
          byte lnght = (thisBright[0] < thisBright[1] ? 5 : map(thisBright[0], thisBright[1]+20, thisBright[1]+100, 5, 30)); 
          if (lnght > 30) lnght = 30; else if (lnght < 2) lnght = 1;
          byte color = random(255);
          for (int xi = i; (xi <= i + lnght || xi < NUM_LEDS); xi++){
            leds[xi] = CHSV(color, 255, 255);
          }
          i+=lnght;
        }
        delay(colorMusicFlash == 1 ? 30 : 120);
        break;
    case 6: //стробоскоп-реакция
      if (thisBright[3] < 10 || thisBright[3] < modes[currentMode].BGBrightness) for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(modes[currentMode].Color[0], modes[currentMode].Color[1], modes[currentMode].BGBrightness);
      else for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(modes[currentMode].Color[0], modes[currentMode].Color[1], thisBright[3]);
      break;
    case 7: //огонь
      fireRoutine();
      delay(40);
      generateValues = true;
      break;
   case 8: //бегущие
        if (colorMusicFlash == 1) leds[NUM_LEDS / 2] = CHSV(random(255), 255, modes[currentMode].Brightness);
      leds[(NUM_LEDS / 2) - 1] = leds[NUM_LEDS / 2];
      if (millis() - running_timer > RUNNING_SPEED) { 
        running_timer = millis();
        for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {
          leds[i] = leds[i + 1];
          leds[NUM_LEDS - i - 1] = leds[i];
        }
      }
      break;
   case 9: //радуга - реакция
      if (millis() - rainbow_timer > 30) {
            rainbow_timer = millis();
            this_color += 9; 
            if (this_color > 255) this_color = 0;
            if (this_color < 0) this_color = 255;
          }
          rainbow_steps = this_color;
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV((int)floor(rainbow_steps), 255, modes[currentMode].Brightness);
            rainbow_steps += thisBright[0] < thisBright[1] ? 0.05 : floatMap((float) thisBright[0], 10.00, (float)thisBright[2], 0.05, RAINBOW_REACTION_STEP);
            if (rainbow_steps > 255) rainbow_steps = 0;
            if (rainbow_steps < 0) rainbow_steps = 255;
          }
          break;
   case 10: //фоновое радуга + реакция
        for (int i = 0; i < NUM_LEDS; i++){
          leds[i] = CHSV((int) map(inoise8(i, noiseY),100, 200, 20, 255), 255, modes[currentMode].Brightness); 
        }
        scrollSpeed = (thisBright[0] < thisBright[1] ? 2 : floatMap((float)thisBright[0], 10.00, (float)thisBright[2], 2.00, 15.00));
        noiseY += (scrollSpeed < 0 ? 0.05 : scrollSpeed);
        //Serial.print(",");
        //Serial.println(scrollSpeed < 0 ? 0.05 : scrollSpeed);
        if (noiseY > 1000000) noiseY = 0;
        break;
  case 11: //фоновое освещение + реакция
        for (int i = 0; i < NUM_LEDS; i++){
          leds[i] = CHSV((int) map(inoise8(i, noiseY),100, 200, modes[currentMode].Color[0]-10, modes[currentMode].Color[0]+15), 255, modes[currentMode].Brightness);
        }
        scrollSpeed = (thisBright[0] < thisBright[1] ? 2 : floatMap((float)thisBright[0], 10.00, (float)thisBright[2], 2.00, 15.00));
        noiseY += (scrollSpeed < 0 ? 0.05 : scrollSpeed);
        //Serial.print(",");
        //Serial.println(scrollSpeed < 0 ? 0.05 : scrollSpeed);
        if (noiseY > 1000000) noiseY = 0;
        break;
  case 12: //фоновое радуга
        for (int i = 0; i < NUM_LEDS; i++){
          leds[i] = CHSV((int) map(inoise8(i, noiseY),100, 200, 20, 255), 255, modes[currentMode].Brightness); 
        }
        noiseY += BG_SCROLL_SPEED; //speed, 0 - slow, 15 - fast 
        //Serial.print(",");
        //Serial.println(BG_SCROLL_SPEED);
        if (noiseY > 1000000) noiseY = 0;
        break;
  case 13: //фоновое освещение
        for (int i = 0; i < NUM_LEDS; i++){
          leds[i] = CHSV((int) map(inoise8(i, noiseY),100, 200, modes[currentMode].Color[0]-10, modes[currentMode].Color[0]+15), 255, modes[currentMode].Brightness);
        }
        noiseY += BG_SCROLL_SPEED; //speed, 0 - slow, 15 - fast 
        //Serial.print(",");
        //Serial.println(BG_SCROLL_SPEED);
        if (noiseY > 1000000) noiseY = 0;
        break;
  case 14: //стробоскоп - изменения цвета
        if (millis() - color_timer > COLOR_SPEED) {
                  color_timer = millis();
                  this_color += COLOR_SPEED > 100 ? 1 : map(COLOR_SPEED, 100, 0, 1, 10);
                  if (this_color > 255) this_color = 0;
        }
        if (thisBright[3] < 10) for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, modes[currentMode].Color[1], modes[currentMode].BGBrightness); 
        else for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(this_color, modes[currentMode].Color[1], thisBright[3]);
        break;
  }
}


void fireRoutine()
{
    for (int i = 0; i < NUM_LEDS / 2; i++){
      leds[i] = CHSV(map(i, 0, NUM_LEDS / 2, 0, 15), 255, random(101)>map(i, 0, NUM_LEDS / 2, 0, 101) ? map(i, 0, NUM_LEDS / 2, 100, 0) : map(i, 0, NUM_LEDS / 2, 0, modes[currentMode].Brightness));
    }
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) leds[i] = CHSV(map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, 15, 0), 255, random(101)>map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, 101, 0) ? map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, 100, 00) : map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, modes[currentMode].Brightness, 0));
}
