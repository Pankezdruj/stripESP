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

    else if (!strncmp_P(inputBuffer, PSTR("GET"), 3))
    {
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("EFF"), 3))
    {
      EepromManager::SaveModesSettings(&currentMode, modes);
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      this_mode = (uint8_t)atoi(buff);
      eeprom_timer = millis();
      eeprom_flag = true;
      delay(1);
      sendCurrent(inputBuffer);
      FastLED.setBrightness(modes[this_mode].Brightness);
      Serial.println(this_mode);
    }

    else if (!strncmp_P(inputBuffer, PSTR("BRI"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[this_mode].Brightness = constrain(atoi(buff), 1, 255);
      FastLED.setBrightness(modes[this_mode].Brightness);
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }
    else if (!strncmp_P(inputBuffer, PSTR("BRI_BG"), 6))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[this_mode].BGBrightness = constrain(atoi(buff), 1, 255);
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("CLR"), 3))
    {
      memcpy(buff, &inputBuffer[3], 3);   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[this_mode].Color[0] = constrain(atoi(buff), 1, 255);
      memcpy(buff, &inputBuffer[7], strlen(inputBuffer));
      modes[this_mode].Color[1] = constrain(atoi(buff), 1, 255);
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }


    else if (!strncmp_P(inputBuffer, PSTR("SPD"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[this_mode].Speed = atoi(buff);
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }
    else if (!strncmp_P(inputBuffer, PSTR("P_ON"), 4))
    {
      ONstate = true; 
      FastLED.clear(); 
      FastLED.show(); 
      updateEEPROM();
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_OFF"), 5))
    {
      ONstate = false; 
      FastLED.clear(); 
      FastLED.show(); 
      updateEEPROM();
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("CAL"), 3))
    {
      fullLowPass();
      eepromTimeout = millis();
    }
    else if (!strncmp_P(inputBuffer, PSTR("RAVE"), 4))
    {
      memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      RAVE_MODE = (atoi(buff) == 1 ? true : false);
      eeprom_timer = millis();
      eeprom_flag = true;
      sendCurrent(inputBuffer);
    }
    

    
void sendCurrent(char *outputBuffer)
{
  sprintf_P(outputBuffer, PSTR("CURR %u %u %u %u %u %u %u %u %u %u %u"),
    EEPROM.read(EEPROM_LAMP_ID_ADRESS),
    LAMP_TYPE,
    this_mode,
    modes[this_mode].Brightness == 10 ? 0 : modes[currentMode].Brightness,
    modes[this_mode].BGBrightness,
    modes[this_mode].Speed,
    modes[this_mode].Color[0],
    modes[this_mode].Color[1],
    RAVE_MODE,
    ONstate,
    espMode
    );
  sprintf_P(outputBuffer, PSTR("%s %f"), outputBuffer, VERSION);
}

}
