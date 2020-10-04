uint32_t effTimer;

void effectsTick()
{
  if (!dawnFlag)
  {
    if (ONflag && (millis() - effTimer >= ((currentMode < 5 || currentMode > 13) ? modes[currentMode].Speed : 50)))
    {
      effTimer = millis();
      switch (currentMode)
      {
        case EFF_FIRE:           fireRoutine(true);           break;
        case EFF_WATERFALL:      fire2012WithPalette();       break;
        case EFF_RAINBOW_VER:    rainbowVerticalRoutine();    break;
        case EFF_RAINBOW_DIAG:   rainbowDiagonalRoutine();    break;
        case EFF_COLORS:         colorsRoutine();             break;
        case EFF_CLOUDS:         cloudsNoiseRoutine();        break;
        case EFF_LAVA:           lavaNoiseRoutine();          break;
        case EFF_PLASMA:         plasmaNoiseRoutine();        break;
        case EFF_RAINBOW_STRIPE: rainbowStripeNoiseRoutine(); break;
        case EFF_ZEBRA:          zebraNoiseRoutine();         break;
        case EFF_NOISE:          RainbowCometRoutine();       break;
        case EFF_COLOR:          colorRoutine();              break;
        case EFF_SNOW:           snowRoutine();               break;
        case EFF_SNOWSTORM:      snowStormRoutine();          break;
        case EFF_MATRIX:         matrixRoutine();             break;
        case EFF_LIGHTERS:       lightersRoutine();           break;
        case EFF_WHITE_COLOR:    whiteColorStripeRoutine();   break;
        default:                                              break;
      }
      FastLED.show();
    }
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

  #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)          // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
  #endif
  
  TimerManager::TimerRunning = false;
  TimerManager::TimerHasFired = false;
  TimerManager::TimeToFire = 0ULL;

  #if (USE_MQTT)
  if (espMode == 1U)
  {
    MqttManager::needToPublish = true;
  }
  #endif
}

void setDefaultParams(){
 modes[0].Speed = 95;
 modes[0].Color[0] = 1;
 modes[0].Color[1] = 255;

 modes[1].Speed = 75;
 modes[1].Color[0] = 170;
 modes[1].Color[1] = 255;

 modes[2].Speed = 25;
 modes[2].Scale = 20;

 modes[3].Scale = 30;

 modes[4].Scale = 5;

 modes[5].Speed = 240;
 modes[5].Scale = 40;

 modes[6].Speed = 240;
 modes[6].Scale = 40;

 modes[7].Speed = 233;
 modes[7].Scale = 26;

 modes[8].Speed = 250;
 modes[8].Scale = 25;

 modes[9].Speed = 250;
 modes[9].Scale = 28;
 modes[9].Color[0] = 35;
 modes[9].Color[1] = 0;

 modes[10].Color[0] = 0;
 modes[10].Color[1] = 255;

 modes[11].Color[0] = 35;
 modes[11].Color[1] = 0;

 modes[12].Color[0] = 35;
 modes[12].Color[1] = 0;

 modes[15].Scale = 40;

 modes[16].Speed = 100;
 modes[16].Scale = 100;
}
