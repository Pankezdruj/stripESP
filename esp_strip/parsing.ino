void parseUDP()
{
  int32_t packetSize = Udp.parsePacket();

  if (packetSize)
  {
    int16_t n = Udp.read(packetBuffer, MAX_UDP_BUFFER_SIZE);
    packetBuffer[n] = '\0';
    strcpy(inputBuffer, packetBuffer);

    #ifdef GENERAL_DEBUG
    LOG.print(F("Inbound UDP packet: "));
    LOG.println(inputBuffer);
    #endif

    if (Udp.remoteIP() == WiFi.localIP())                   // не реагировать на свои же пакеты
    {
      return;
    }

    char reply[MAX_UDP_BUFFER_SIZE];
    processInputBuffer(inputBuffer, reply, true);
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();

    #ifdef GENERAL_DEBUG
    LOG.print(F("Outbound UDP packet: "));
    LOG.println(reply);
    LOG.println();
    #endif
  }
}


void processInputBuffer(char *inputBuffer, char *outputBuffer, bool generateOutput)
{
    char buff[MAX_UDP_BUFFER_SIZE], *endToken = NULL;

    if (!strncmp_P(inputBuffer, PSTR("GET"), 3))
    {
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("EFF"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      currentMode = (uint8_t)atoi(buff);
      EepromManager::SaveModesSettings(&currentMode, modes);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      FastLED.clear();
      delay(1);
      sendCurrent(inputBuffer);
      FastLED.setBrightness(getBrightnessU());
      settingsTick();
    }

    else if (!strncmp_P(inputBuffer, PSTR("BRI"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Brightness = constrain(atoi(buff), 1, 255);
      EepromManager::SaveModesSettings(&currentMode, modes);
      FastLED.setBrightness(getBrightnessU());
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
      settingsTick();
    }

    else if (!strncmp_P(inputBuffer, PSTR("BGB"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].BGBrightness = atoi(buff);
      EepromManager::SaveModesSettings(&currentMode, modes);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
      settingsTick();
    }

    else if (!strncmp_P(inputBuffer, PSTR("CLR"), 3))
    {
      memcpy(buff, &inputBuffer[3], 3);   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Color[0] = constrain(atoi(buff), 1, 255);
      memcpy(buff, &inputBuffer[7], strlen(inputBuffer));
      modes[currentMode].Color[1] = constrain(atoi(buff), 1, 255);
      EepromManager::SaveModesSettings(&currentMode, modes);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
      settingsTick();
    }


    else if (!strncmp_P(inputBuffer, PSTR("SPD"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Speed = atoi(buff);
      EepromManager::SaveModesSettings(&currentMode, modes);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
      settingsTick();
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_ON"), 4))
    {
      ONflag = true;
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      changePower();
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_OFF"), 5))
    {
      ONflag = false;
      settChanged = true;
      eepromTimeout = millis();
      changePower();
      sendCurrent(inputBuffer);
    }
    else if (!strncmp_P(inputBuffer, PSTR("CAL"), 3))
    {
      fullLowPass();
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
    }
    else if (!strncmp_P(inputBuffer, PSTR("RAVE"), 4))
    {
      memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      RAVE_MODE = (atoi(buff) == 1 ? true : false);
      EepromManager::SaveRaveMode(RAVE_MODE);
      if(RAVE_MODE) {
          modes[1].Speed = 100; //изменение цвета - скорость 
          modes[2].Speed = 100; //радуга - скорость 
          modes[3].Speed = 100; //радуга-громкость - резкость + скорость 
          modes[6].Speed = 70; //стробоскоп - резкость
          modes[9].Speed = 100; //радуга-реакция - резкость
          modes[12].Speed = 100; //фоновая радуга - скорость
          modes[13].Speed = 100; //фоновое освещение - скорость
          modes[14].Speed = 70; //стробоскоп изменение - резкость
        } else {
          modes[1].Speed = 60; //изменение цвета - скорость 
          modes[2].Speed = 0; //радуга - скорость 
          modes[3].Speed = 0; //радуга-громкость - резкость + скорость 
          modes[6].Speed = 5; //стробоскоп - резкость
          modes[9].Speed = 8; //радуга-реакция - резкость
          modes[12].Speed = 10; //фоновая радуга - скорость
          modes[13].Speed = 10; //фоновое освещение - скорость
          modes[14].Speed = 5; //стробоскоп изменение - резкость
        }
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
      settingsTick();
    }
    else if (!strncmp_P(inputBuffer, PSTR("RE"), 2))
    {
      memcpy(buff, &inputBuffer[2], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      REACTION = atoi(buff);
      EepromManager::SaveReaction(REACTION);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
    }
    else
    {
      inputBuffer[0] = '\0';
    }

    if (strlen(inputBuffer) <= 0)
    {
      return;
    }

    if (generateOutput)                                     // если запрошен вывод ответа выполнения команд, копируем его в исходящий буфер
    {
      strcpy(outputBuffer, inputBuffer);
    }
    inputBuffer[0] = '\0';                                  // очистка буфера, читобы не он не интерпретировался, как следующий входной пакет
}

void sendCurrent(char *outputBuffer)
{
  sprintf_P(outputBuffer, PSTR("CURR %u %u %u %u %u %u %u %u %u %u %u %u"),
    EEPROM.read(EEPROM_LAMP_ID_ADRESS),
    LAMP_TYPE,
    currentMode,
    modes[currentMode].Brightness,
    modes[currentMode].BGBrightness,
    modes[currentMode].Speed,
    modes[currentMode].Color[0],
    modes[currentMode].Color[1],
    RAVE_MODE,
    REACTION,
    ONflag,
    espMode
    );
  sprintf_P(outputBuffer, PSTR("%s %f"), outputBuffer, VERSION);
}
