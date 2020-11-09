void settingsTick() {
  switch(currentMode) {
    case(1): //изменение цвета
        COLOR_SPEED = map(modes[1].Speed, 0, 100, 255, 0);
        break;
    case(2): //радуга
        RAINBOW_STEP_2 = floatMap((float)modes[2].Speed, 0.00, 100.00, 1.00, 10.00); //ширина полосок
        RAINBOW_PERIOD = map(modes[2].Speed, 0, 100, 1, 20); //скорость
        break;
    case(3): //радуга - громкость 
        SMOOTH = floatMap((float)modes[3].Speed, 0.00, 100.00, 0.05, 1.00);
        RAINBOW_STEP = floatMap((float)modes[3].Speed, 0.00, 100.00, 3.50, 20.00);
        break;
    case(6): //стробоскоп
        SMOOTH_STEP = map(modes[6].Speed, 0, 100, 1, 255);
        break;
    case(9): //радуга - реакция 
        RAINBOW_REACTION_STEP = floatMap((float)modes[9].Speed, 0.00, 100.00, 0.06, 2.00);
        break;
    case(12): //фоновая радуга 
        BG_SCROLL_SPEED = floatMap((float)modes[12].Speed, 0.00, 100.00, 0.00, 15.00);
        break;
    case(13): //фоновое освещение 
        BG_SCROLL_SPEED = floatMap((float)modes[13].Speed, 0.00, 100.00, 0.00, 15.00);
        break;
    case(14): //стробоскоп-изменение
        COLOR_SPEED = map(modes[14].Speed, 0, 100, 255, 0);
        SMOOTH_STEP = map(modes[14].Speed, 0, 100, 1, 255);
        break;
  }
  //FastLED.setBrightness(modes[currentMode].Brightness);
}
