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

    #if (USE_MQTT)                                          // отправка ответа выполнения команд по MQTT, если разрешено
    if (espMode == 1U)
    {
      strcpy(MqttManager::mqttBuffer, reply);               // разрешение определяется при выполнении каждой команды отдельно, команды GET, DEB, DISCOVER и OTA, пришедшие по UDP, игнорируются (приходят раз в 2 секунды от приложения)
    }
    #endif
    
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

    if (!strncmp_P(inputBuffer, PSTR("DEB"), 3))
    {
        #ifdef USE_NTP
        getFormattedTime(inputBuffer);
        sprintf_P(inputBuffer, PSTR("OK %s"), inputBuffer);
        #else
        strcpy_P(inputBuffer, PSTR("OK --:--"));
        #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("GET"), 3))
    {
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("EFF"), 3))
    {
      EepromManager::SaveModesSettings(&currentMode, modes);
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      currentMode = (uint8_t)atoi(buff);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      FastLED.clear();
      delay(1);
      sendCurrent(inputBuffer);
      FastLED.setBrightness(modes[currentMode].Brightness);
      Serial.println(currentMode);
      switch(currentMode) {
        case 0: modes[currentMode].Speed = 95; break;
        case 1: modes[currentMode].Speed = 75; break;
        case 4: modes[currentMode].Speed = 65; break;
        case 10: modes[currentMode].Speed = 55; break;
        case 14: modes[currentMode].Speed = 75; break;
        case 15: modes[currentMode].Speed = 55; break;
      }

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("BRI"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Brightness = constrain(atoi(buff), 1, 255);
      FastLED.setBrightness(modes[currentMode].Brightness);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("DEF"), 3))
    {
      setDefaultParams();
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("CLR"), 3))
    {
      memcpy(buff, &inputBuffer[3], 3);   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Color[0] = constrain(atoi(buff), 1, 255);
      memcpy(buff, &inputBuffer[7], strlen(inputBuffer));
      modes[currentMode].Color[1] = constrain(atoi(buff), 1, 255);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }


    else if (!strncmp_P(inputBuffer, PSTR("SPD"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Speed = atoi(buff);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("SCA"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Scale = atoi(buff);
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_ON"), 4))
    {
      ONflag = true;
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();
      changePower();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_OFF"), 5))
    {
      ONflag = false;
      settChanged = true;
      eepromTimeout = millis();
      changePower();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("ALM_SET"), 7))
    {
      uint8_t alarmNum = (char)inputBuffer[7] - '0';
      alarmNum -= 1;
      if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 9)
      {
        alarms[alarmNum].State = true;
        sendAlarms(inputBuffer);
      }
      else if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 9)
      {
        alarms[alarmNum].State = false;
        sendAlarms(inputBuffer);
      }
      else
      {
        memcpy(buff, &inputBuffer[8], strlen(inputBuffer)); // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 9
        alarms[alarmNum].Time = atoi(buff);
        sendAlarms(inputBuffer);
      }
      EepromManager::SaveAlarmsSettings(&alarmNum, alarms);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        strcpy(MqttManager::mqttBuffer, inputBuffer);
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("ALM_GET"), 7))
    {
      sendAlarms(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("DAWN"), 4))
    {
      memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 5
      dawnMode = atoi(buff) - 1;
      EepromManager::SaveDawnMode(&dawnMode);
      sendAlarms(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("DISCOVER"), 8))  // обнаружение приложением модуля esp в локальной сети
    {
      if (espMode == 1U)                                    // работает только в режиме WiFi клиента
      {
        sendCurrent(inputBuffer);
      }
    }

    else if (!strncmp_P(inputBuffer, PSTR("TMR_GET"), 7))
    {
      sendTimer(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("TMR_SET"), 7))
    {
      memcpy(buff, &inputBuffer[8], 2);                     // взять подстроку, состоящую из 9 и 10 символов, из строки inputBuffer
      TimerManager::TimerRunning = (bool)atoi(buff);

      memcpy(buff, &inputBuffer[10], strlen(inputBuffer));  // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 13
      TimerManager::TimeToFire = millis() + (strtoull(buff, &endToken, 10) * 1000);

      TimerManager::TimerHasFired = false;
      sendTimer(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("OTA"), 3))
    {
      #ifdef OTA
      otaManager.RequestOtaUpdate();
      delay(50);
      otaManager.RequestOtaUpdate();
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
      FastLED.clear();
      delay(1);
      ONflag = true;
      changePower();
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("BTN"), 3))
    {
      if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 4)
      {
        buttonEnabled = true;
        EepromManager::SaveButtonEnabled(&buttonEnabled);
        sendCurrent(inputBuffer);
      }
      else if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 4)
      {
        buttonEnabled = false;
        EepromManager::SaveButtonEnabled(&buttonEnabled);
        sendCurrent(inputBuffer);
      }

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        strcpy(MqttManager::mqttBuffer, inputBuffer);
        MqttManager::needToPublish = true;
      }
      #endif
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
  sprintf_P(outputBuffer, PSTR("CURR %u %u %u %u %u %u %u %u %u %u"),
    EEPROM.read(EEPROM_LAMP_ID_ADRESS),
    LAMP_TYPE,
    currentMode,
    modes[currentMode].Brightness == 10 ? 0 : modes[currentMode].Brightness,
    modes[currentMode].Speed,
    modes[currentMode].Scale,
    ONflag,
    espMode,
    modes[currentMode].Color[0],
    modes[currentMode].Color[1]
    );
  
  #ifdef USE_NTP
  strcat_P(outputBuffer, PSTR(" 1"));
  #else
  strcat_P(outputBuffer, PSTR(" 0"));
  #endif

  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)TimerManager::TimerRunning);
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)buttonEnabled);

  #ifdef USE_NTP
  char timeBuf[9];
  getFormattedTime(timeBuf);
  sprintf_P(outputBuffer, PSTR("%s %s"), outputBuffer, timeBuf);
  #else
  time_t currentTicks = millis() / 1000UL;
  sprintf_P(outputBuffer, PSTR("%s %02u:%02u:%02u"), outputBuffer, VERSION, hour(currentTicks), minute(currentTicks), second(currentTicks));
  #endif
  sprintf_P(outputBuffer, PSTR("%s %f"), outputBuffer, VERSION);
}

void sendAlarms(char *outputBuffer)
{
  strcpy_P(outputBuffer, PSTR("ALMS"));

  for (byte i = 0; i < 7; i++)
  {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)alarms[i].State);
  }

  for (byte i = 0; i < 7; i++)
  {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, alarms[i].Time);
  }

  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, dawnMode);
}

void sendTimer(char *outputBuffer)
{
  sprintf_P(outputBuffer, PSTR("TMR %u %u"),
    TimerManager::TimerRunning,
   (TimerManager::TimerRunning ? (int)floor((TimerManager::TimeToFire - millis()) / 1000) : 0));
   
}
