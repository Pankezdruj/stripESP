
// --- ИНИЦИАЛИЗАЦИЯ ОБЪЕКТОВ ----------
WiFiManager wifiManager;
WiFiServer wifiServer(ESP_HTTP_PORT);
WiFiUDP Udp;



// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_UDP_BUFFER_SIZE];                     // buffer to hold incoming packet
char inputBuffer[MAX_UDP_BUFFER_SIZE];


bool loadingFlag = true;
bool ONflag = false;
bool settChanged = false;

bool CaptivePortalManager::captivePortalCalled = false;


void setup()
{
  Serial.begin(115200);
  ESP.wdtEnable(WDTO_8S);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(BRIGHTNESS);


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


 

  // WI-FI
  wifiManager.setDebugOutput(WIFIMAN_DEBUG);                // вывод отладочных сообщений
  // wifiManager.setMinimumSignalQuality();                 // установка минимально приемлемого уровня сигнала WiFi сетей (8% по умолчанию)
  CaptivePortalManager *captivePortalManager = new CaptivePortalManager(&wifiManager);
  if (espMode == 0U)                                        // режим WiFi точки доступа
  {
    // wifiManager.setConfigPortalBlocking(false);

    if (sizeof(AP_STATIC_IP))
    {
      LOG.println(F("Используется статический IP адрес WiFi точки доступа"));
      wifiManager.setAPStaticIPConfig(                      // wifiManager.startConfigPortal использовать нельзя, т.к. он блокирует вычислительный процесс внутри себя, а затем перезагружает ESP, т.е. предназначен только для ввода SSID и пароля
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]),      // IP адрес WiFi точки доступа
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], 1),                    // первый доступный IP адрес сети
        IPAddress(255, 255, 255, 0));                                                       // маска подсети
    }

    WiFi.softAP(AP_NAME, AP_PASS);

    LOG.println(F("Старт в режиме WiFi точки доступа"));
    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.softAPIP());

    wifiServer.begin();
  }
  else                                                      // режим WiFi клиента (подключаемся к роутеру, если есть сохранённые SSID и пароль, иначе создаём WiFi точку доступа и запрашиваем их)
  {
    LOG.println(F("Старт в режиме WiFi клиента (подключение к роутеру)"));

    if (WiFi.SSID().length())
    {
      LOG.printf_P(PSTR("Подключение к WiFi сети: %s\n"), WiFi.SSID().c_str());

      if (sizeof(STA_STATIC_IP))                            // ВНИМАНИЕ: настраивать статический ip WiFi клиента можно только при уже сохранённых имени и пароле WiFi сети (иначе проявляется несовместимость библиотек WiFiManager и WiFi)
      {
        LOG.print(F("Сконфигурирован статический IP адрес: "));
        LOG.printf_P(PSTR("%u.%u.%u.%u\n"), STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]);
        wifiManager.setSTAStaticIPConfig(
          IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]),// статический IP адрес ESP в режиме WiFi клиента
          IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], 1),               // первый доступный IP адрес сети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
          IPAddress(255, 255, 255, 0));                                                     // маска подсети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
      }
    }
    else
    {
      LOG.println(F("WiFi сеть не определена, запуск WiFi точки доступа для настройки параметров подключения к WiFi сети..."));
      CaptivePortalManager::captivePortalCalled = true;
      wifiManager.setBreakAfterConfig(true);                // перезагрузка после ввода и сохранения имени и пароля WiFi сети
      showWarning(CRGB::Yellow, 1000U, 500U);               // мигание жёлтым цветом 0,5 секунды (1 раз) - нужно ввести параметры WiFi сети для подключения
    }

    wifiManager.setConnectTimeout(ESP_CONN_TIMEOUT);        // установка времени ожидания подключения к WiFi сети, затем старт WiFi точки доступа
    wifiManager.setConfigPortalTimeout(ESP_CONF_TIMEOUT);   // установка времени работы WiFi точки доступа, затем перезагрузка; отключить watchdog?
    wifiManager.autoConnect(AP_NAME, AP_PASS);              // пытаемся подключиться к сохранённой ранее WiFi сети; в случае ошибки, будет развёрнута WiFi точка доступа с указанными AP_NAME и паролем на время ESP_CONN_TIMEOUT секунд; http://AP_STATIC_IP:ESP_HTTP_PORT (обычно http://192.168.0.1:80) - страница для ввода SSID и пароля от WiFi сети роутера

    delete captivePortalManager;
    captivePortalManager = NULL;

    if (WiFi.status() != WL_CONNECTED)                      // подключение к WiFi не установлено
    {
      if (CaptivePortalManager::captivePortalCalled)        // была показана страница настройки WiFi ...
      {
        if (millis() < (ESP_CONN_TIMEOUT + ESP_CONF_TIMEOUT) * 1000U) // пользователь ввёл некорректное имя WiFi сети и/или пароль или запрошенная WiFi сеть недоступна
        {
          LOG.println(F("Не удалось подключиться к WiFi сети\nУбедитесь в корректности имени WiFi сети и пароля\nРестарт для запроса нового имени WiFi сети и пароля...\n"));
          wifiManager.resetSettings();
        }
        else                                                // пользователь не вводил имя WiFi сети и пароль
        {
          LOG.println(F("Время ожидания ввода SSID и пароля от WiFi сети или подключения к WiFi сети превышено\nЛампа будет перезагружена в режиме WiFi точки доступа!\n"));


          LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\nНовый рабочий режим: ESP_MODE = %d, %s\nРестарт...\n"),
            espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
        }
      }
      else                                                  // страница настройки WiFi не была показана, не удалось подключиться к ранее сохранённой WiFi сети (перенос в новую WiFi сеть)
      {
        LOG.println(F("Не удалось подключиться к WiFi сети\nВозможно, заданная WiFi сеть больше не доступна\nРестарт для запроса нового имени WiFi сети и пароля...\n"));
        wifiManager.resetSettings();
      }

      showWarning(CRGB::Red, 1000U, 500U);                  // мигание красным цветом 0,5 секунды (1 раз) - ожидание ввода SSID'а и пароля WiFi сети прекращено, перезагрузка
      ESP.restart();
    }

    if (CaptivePortalManager::captivePortalCalled &&        // первое подключение к WiFi сети после настройки параметров WiFi на странице настройки - нужна перезагрузка для применения статического IP
        sizeof(STA_STATIC_IP) &&
        WiFi.localIP() != IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]))
    {
      LOG.println(F("Рестарт для применения заданного статического IP адреса..."));
      delay(100);
      ESP.restart();
    }

    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.localIP());
  }
  ESP.wdtFeed();

  LOG.printf_P(PSTR("Порт UDP сервера: %u\n"), localPort);
  Udp.begin(localPort);
  
}


void loop()
{
  mainLoop();       // главный цикл обработки и отрисовки
  eepromTick();     // проверка не пора ли сохранить настройки
  parseUDP();
  settingsTick();
  ESP.wdtFeed();                                            // пнуть собаку
}
