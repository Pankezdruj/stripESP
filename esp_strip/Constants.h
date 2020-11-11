#pragma once

#include <ESP8266WebServer.h>

//#define LAMP_ID               (2)
#define LAMP_TYPE             (4)                           // 1 - small lamp, 2 - giant lamp, 3 - board lamp
#define VERSION               (1.0)

// ============= НАСТРОЙКИ =============
// --- ESP -----------------------------
#define ESP_MODE              (1U)                          // 0U - WiFi точка доступа, 1U - клиент WiFi (подключение к роутеру)
uint8_t espMode = 1U;                                 // ESP_MODE может быть сохранён в энергонезависимую память и изменён в процессе работы лампы без необходимости её перепрошивки

#define ESP_RESET_ON_START    (false)                       // true - если при старте нажата кнопка (или кнопки нет!), сохранённые настройки будут сброшены; false - не будут
#define ESP_HTTP_PORT         (80U)                         // номер порта, который будет использоваться во время первой утановки имени WiFi сети (и пароля), к которой потом будет подключаться лампа в режиме WiFi клиента (лучше не менять)
#define ESP_UDP_PORT          (8888U)                       // номер порта, который будет "слушать" UDP сервер во время работы лампы как в режиме WiFi точки доступа, так и в режиме WiFi клиента (лучше не менять)
#define ESP_CONN_TIMEOUT      (7U)                          // время в секундах (ДОЛЖНО БЫТЬ МЕНЬШЕ 8, иначе сработает WDT), которое ESP будет пытаться подключиться к WiFi сети, после его истечения автоматически развернёт WiFi точку доступа
#define ESP_CONF_TIMEOUT      (300U)                        // время в секундах, которое ESP будет ждать ввода SSID и пароля WiFi сети роутера в конфигурационном режиме, после его истечения ESP перезагружается
#define GENERAL_DEBUG                                       // если строка не закомментирована, будут выводиться отладочные сообщения
#define WIFIMAN_DEBUG         (false)                       // вывод отладочных сообщений при подключении к WiFi сети: true - выводятся, false - не выводятся; настройка не зависит от GENERAL_DEBUG
#define LED_PIN               (2U)                          // пин ленты                (D4)
#define SOUND_R               A0                           // аналоговый пин вход аудио, правый канал
#define MLED_PIN              (13U)                        // пин светодиода режимов
#define MLED_ON               HIGH


// --- ESP (WiFi клиент) ---------------
const uint8_t STA_STATIC_IP[] = {};                         // статический IP адрес: {} - IP адрес определяется роутером; {192, 168, 1, 66} - IP адрес задан явно (если DHCP на роутере не решит иначе); должен быть из того же диапазона адресов, что разадёт роутер
                                                            // SSID WiFi сети и пароль будут запрошены WiFi Manager'ом в режиме WiFi точки доступа, нет способа захардкодить их в прошивке

// --- AP (WiFi точка доступа) ---
#define AP_NAME               ("ExoyIndividual")                   // имя WiFi точки доступа, используется как при запросе SSID и пароля WiFi сети роутера, так и при работе в режиме ESP_MODE = 0
#define AP_PASS               ("12345678")                  // пароль WiFi точки доступа
const uint8_t AP_STATIC_IP[] = {192, 168, 4, 1};            // статический IP точки доступа (лучше не менять)



// --- ЛЕНТА -------------------------
#define BRIGHTNESS            (255U)                         // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT         (5000U)                       // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define NUM_LEDS              (300U)

#define COLOR_ORDER           (GRB)                         // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB


#define MAX_UDP_BUFFER_SIZE   (129U)                        // максимальный размер буффера UDP сервера

#define LOG                   Serial
#define MODE_AMOUNT           (15U)


// --- БИБЛИОТЕКИ ----------------------
#define FASTLED_INTERRUPT_RETRY_COUNT   (0)                 // default: 2; // Use this to determine how many times FastLED will attempt to re-transmit a frame if interrupted for too long by interrupts
//#define FASTLED_ALLOW_INTERRUPTS      (1)                 // default: 1; // Use this to force FastLED to allow interrupts in the clockless chipsets (or to force it to disallow), overriding the default on platforms that support this. Set the value to 1 to allow interrupts or 0 to disallow them.
#define FASTLED_ESP8266_RAW_PIN_ORDER                       // FASTLED_ESP8266_RAW_PIN_ORDER, FASTLED_ESP8266_D1_PIN_ORDER or FASTLED_ESP8266_NODEMCU_PIN_ORDER



bool buttonEnabled = false;


// ----- настройки радуги
float RAINBOW_STEP = 5.00;         // шаг изменения цвета радуги

// ----- отрисовка
#define MAIN_LOOP 5               // период основного цикла отрисовки (по умолчанию 5)

// ----- сигнал
#define EXP 1.0                  // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)

// ----- нижний порог шумов
uint16_t LOW_PASS = 100;          // нижний порог шумов режим VU, ручная настройка
#define EEPROM_LOW_PASS 1         // порог шумов хранится в энергонезависимой памяти (по умолч. 1)
#define LOW_PASS_ADD 13           // "добавочная" величина к нижнему порогу, для надёжности (режим VU)

// ----- режим шкала громкости
float SMOOTH = 0.5;               // коэффициент плавности анимации VU (по умолчанию 0.5)
#define MAX_COEF 1.8              // коэффициент громкости (максимальное равно среднему * этот коэф) (по умолчанию 1.8)

// ----- режим цветомузыки
byte SMOOTH_STEP = 1;           // шаг уменьшения яркости в режиме цветомузыки (чем больше, тем быстрее гаснет)
float RAINBOW_REACTION_STEP = 1;

// ----- режим подсветки
byte LIGHT_COLOR = 0;             // начальный цвет подсветки
byte LIGHT_SAT = 255;             // начальная насыщенность подсветки
byte COLOR_SPEED = 1;
int RAINBOW_PERIOD = 1;
float RAINBOW_STEP_2 = 0.5;
float BG_SCROLL_SPEED = 5;

// ----- режим бегущих частот
byte RUNNING_SPEED = 11;
int noiseY = 0;

//----- огонь
bool generateValues = true;

bool RAVE_MODE = 1;
bool REACTION = 1;


int Rlenght, Llenght;
float RsoundLevel, RsoundLevel_f = 0;
float LsoundLevel, LsoundLevel_f = 0;

float averageLevel = 50;
int maxLevel = 100;
int MAX_CH = NUM_LEDS / 2;
int hue;
unsigned long main_timer, running_timer, color_timer, rainbow_timer, eeprom_timer;
float averK = 0.006;
byte count;
float ind = (float)255 / MAX_CH;   // коэффициент перевода для палитры
int RcurrentLevel, LcurrentLevel;
boolean colorMusicFlash;
int thisBright[4];
float rainbow_steps;
int this_color;
