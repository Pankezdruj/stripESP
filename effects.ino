// ============= ЭФФЕКТЫ ===============
// функция плавного угасания цвета для всех пикселей
void fader(uint8_t step)
{
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      fadePixel(i, j, step);
    }
  }
}

void fadePixel(uint8_t i, uint8_t j, uint8_t step)          // новый фейдер
{
  int32_t pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0U) return;

  if (leds[pixelNum].r >= 30U ||
      leds[pixelNum].g >= 30U ||
      leds[pixelNum].b >= 30U)
  {
    leds[pixelNum].fadeToBlackBy(step);
  }
  else
  {
    leds[pixelNum] = 0U;
  }
}

// =============- водопад -===============
#define COOLINGNEW 32
#define SPARKINGNEW 80 
extern const TProgmemRGBPalette16 WaterfallColors_p FL_PROGMEM = {
  0x000000, 0x060707, 0x101110, 0x151717,
  0x1C1D22, 0x242A28, 0x363B3A, 0x313634,
  0x505552, 0x6B6C70, 0x98A4A1, 0xC1C2C1,
  0xCACECF, 0xCDDEDD, 0xDEDFE0, 0xB2BAB9
};

void fire2012WithPalette(){
    static byte heat[WIDTH][HEIGHT];

    for(uint8_t x = 0; x < WIDTH; x++) {
      // Step 1.  Cool down every cell a little
      for (int i = 0; i < HEIGHT; i++) {
          heat[x][i] = qsub8(heat[x][i], random8(0, ((COOLINGNEW * 10) / HEIGHT) + 2));
      }

      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for (int k = HEIGHT - 1; k >= 2; k--) {
          heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2]) / 3;
      }

      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if (random8() < SPARKINGNEW) {
          int y = random8(2);
          heat[x][y] = qadd8(heat[x][y], random8(160, 255));
      }

      // Step 4.  Map from heat cells to LED colors
      for (int j = 0; j < HEIGHT; j++) {
          // Scale the heat value from 0-255 down to 0-240
          // for best results with color palettes.
          byte colorindex = scale8(heat[x][j], 240);
            leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(CRGBPalette16( CRGB::Black, CHSV(modes[currentMode].Color[0], modes[currentMode].Color[1], 255U) , CHSV(modes[currentMode].Color[0], modes[currentMode].Color[1], 255U) , CRGB::White), colorindex);
      }
    }
}

// ------------- огонь -----------------
#define SPARKLES              (1U)                     // вылетающие угольки вкл выкл
#define UNIVERSE_FIRE                                  // универсальный огонь 2-в-1 Цветной+Белый

uint8_t line[WIDTH];
uint8_t pcnt = 0U;
uint8_t deltaHue = 16U;                                // текущее смещение пламени (hueMask)
uint8_t shiftHue[HEIGHT];                              // массив дороожки горизонтального смещения пламени (hueMask)
uint8_t deltaValue = 16U;                              // текущее смещение пламени (hueValue)
uint8_t shiftValue[HEIGHT];                            // массив дороожки горизонтального смещения пламени (hueValue)

//these values are substracetd from the generated values to give a shape to the animation
static const uint8_t valueMask[8][16] PROGMEM =
{
  {0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  },
  {0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  },
  {0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  },
  {0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  },
  {32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 },
  {64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 },
  {96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 },
  {128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
static const uint8_t hueMask[8][16] PROGMEM =
{
  {25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25 },
  {25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19 },
  {19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16 },
  {13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13 },
  {11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11 },
  {8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8  },
  {5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5  },
  {1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1  }
};

//void fireRoutine(char *isColored) // <- ******* для прошивки Gunner47 + kDn mod ******* (раскомментить/закоментить) C - цветной огонь, остальное - белый
void fireRoutine(bool isColored) // <- ******* для оригинальной прошивки Gunner47 ******* (раскомментить/закоментить)
{
  if (loadingFlag) {
    loadingFlag = false;
    //FastLED.clear();
    generateLine();
    //memset(matrixValue, 0, sizeof(matrixValue)); без очистки
  }
  if (pcnt >= 30) {                                         // внутренний делитель кадров для поднимающегося пламени
    shiftUp();                                              // смещение кадра вверх
    generateLine();                                         // перерисовать новую нижнюю линию случайным образом
    pcnt = 0;
  }
//  drawFrame(pcnt, (strcmp(isColored, "C") == 0));           // прорисовка экрана
  drawFrame(pcnt, isColored);                              // для прошивки где стоит логический параметр
  pcnt += 25;  // делитель кадров: задает скорость подъема пламени 25/100 = 1/4
}

// Randomly generate the next line (matrix row)
void generateLine() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    line[x] = random(127, 255);                             // заполнение случайным образом нижней линии (127, 255) - менее контрастное, (64, 255) - оригинал
  }
}

void shiftUp() {                                            //подъем кадра
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      uint8_t newX = x % 16U;                               // сократил формулу без доп. проверок
      if (y > 7U) continue;
      matrixValue[y][newX] = matrixValue[y - 1U][newX];     //смещение пламени (только для зоны очага)
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++) {                    // прорисовка новой нижней линии
    uint8_t newX = x % 16U;                                 // сократил формулу без доп. проверок
    matrixValue[0U][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(uint8_t pcnt, bool isColored) {                  // прорисовка нового кадра
  int32_t nextv;
#ifdef UNIVERSE_FIRE                                            // если определен универсальный огонь  
  uint8_t baseHue = (float)modes[currentMode].Color[0];
#else
  uint8_t baseHue = isColored ? 255U : 0U;
#endif
  uint8_t baseSat = modes[currentMode].Color[1];  // вычисление базового оттенка

  
  //first row interpolates with the "next" line
  deltaHue = random(0U, 2U) ? constrain (shiftHue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : shiftHue[0]; // random(0U, 2U)= скорость смещения языков чем больше 2U - тем медленнее
                                                                                                                 // 15U, 17U - амплитуда качания -1...+1 относительно 16U 
                                                            // высчитываем плавную дорожку смещения всполохов для нижней строки
                                                            // так как в последствии координаты точки будут исчисляться из остатка, то за базу можем принять кратную ширину матрицы hueMask
                                                            // ширина матрицы hueMask = 16, поэтому нам нужно получить диапазон чисел от 15 до 17
                                                            // далее к предыдущему значению прибавляем случайную 1 и отнимаем случайную 1 - это позволит плавным образом менять значение смещения
  shiftHue[0] = deltaHue;                                   // заносим это значение в стэк

  deltaValue = random(0U, 3U) ? constrain (shiftValue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : shiftValue[0]; // random(0U, 3U)= скорость смещения очага чем больше 3U - тем медленнее
                                                                                                                       // 15U, 17U - амплитуда качания -1...+1 относительно 16U
  shiftValue[0] = deltaValue;


  for (uint8_t x = 0U; x < WIDTH; x++) {                                          // прорисовка нижней строки (сначала делаем ее, так как потом будем пользоваться ее значением смещения)
    uint8_t newX = x % 16;                                                        // сократил формулу без доп. проверок
            nextv =                                                               // расчет значения яркости относительно valueMask и нижерасположенной строки.
                (((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0)
                - pgm_read_byte(&valueMask[0][(x + deltaValue) % 16U]);
    CRGB color = CHSV(                                                            // вычисление цвета и яркости пикселя
                   baseHue + pgm_read_byte(&hueMask[0][(x + deltaHue) % 16U]),    // H - смещение всполохов
                   baseSat,                                                       // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                   (uint8_t)max(0, nextv)                                         // V
                 );
    leds[getPixelNumber(x, 0)] = color;                                            // прорисовка цвета очага
  }

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {                                      // прорисовка остальных строк с учетом значения низлежащих
    deltaHue = shiftHue[y];                                                         // извлекаем положение
    shiftHue[y] = shiftHue[y - 1];                                                  // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем
    deltaValue = shiftValue[y];                                                     // извлекаем положение
    shiftValue[y] = shiftValue[y - 1];                                              // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем


    if (y > 8U) {                                                                   // цикл стирания текущей строоки для искр
      for (uint8_t _x = 0U; _x < WIDTH; _x++) {                                     // стираем строчку с искрами (очень не оптимально)
        drawPixelXY(_x, y, 0U);
      }
    }
    for (uint8_t x = 0U; x < WIDTH; x++) {                                          // пересчет координаты x для текущей строки
      uint8_t newX = x % 16U;                                                       // функция поиска позиции значения яркости для матрицы valueMask
      if (y < 8U) {                                                                 // если строка представляет очаг
        nextv =                                                                     // расчет значения яркости относительно valueMask и нижерасположенной строки.
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
            - pgm_read_byte(&valueMask[y][(x + deltaValue) % 16U]);

        CRGB color = CHSV(                                                                  // определение цвета пикселя
                       baseHue + pgm_read_byte(&hueMask[y][(x + deltaHue) % 16U ]),         // H - смещение всполохов
                       baseSat,                                                             // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                       (uint8_t)max(0, nextv)                                               // V
                     );
        leds[getPixelNumber(x, y)] = color;
      }
      else if (y == 8U && SPARKLES) {                                               // если это самая нижняя строка искр - формитуем искорку из пламени
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1U) != 0U) drawPixelXY(x, y, getPixColorXY(x, y - 2U));  // 20 = обратная величина количества искр
        else drawPixelXY(x, y, 0U);
      }
      else if (SPARKLES) {                                                          // если это не самая нижняя строка искр - перемещаем искорку выше
        // старая версия для яркости
        newX = (random(0, 4)) ? x : (x + WIDTH + random(0U, 2U) - random(0U, 2U)) % WIDTH ;   // с вероятностью 1/3 смещаем искорку влево или вправо
        if (getPixColorXY(x, y - 1U) > 0U) drawPixelXY(newX, y, getPixColorXY(x, y - 1U));    // рисуем искорку на новой строчке
      }
    }
  }
}


// ------------- радуга вертикальная ----------------
uint8_t hue;
void rainbowVerticalRoutine()
{
  hue += 4;
  for (uint8_t j = 0; j < HEIGHT; j++)
  {
    CHSV thisColor = CHSV((uint8_t)(hue + j * modes[EFF_RAINBOW_VER].Scale), 255, 255);
    for (uint8_t i = 0U; i < WIDTH; i++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга диагональная -------------
void rainbowDiagonalRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }

  hue += 8;
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      float twirlFactor = 3.0F * (modes[EFF_RAINBOW_DIAG].Scale / 100.0F);      // на сколько оборотов будет закручена матрица, [0..3]
      CRGB thisColor = CHSV((uint8_t)(hue + (float)(WIDTH / HEIGHT * i + j * twirlFactor) * (float)(255 / maxDim)), 255, 255);
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- цвета -----------------
void colorsRoutine()
{
  if (loadingFlag)
  {
    hue += modes[EFF_COLORS].Scale;

    for (uint16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(hue, 255U, 255U);
    }
  }
}

// ------------- цвет ------------------
void colorRoutine()
{
  if (loadingFlag) 
    for (uint16_t i = 0U; i < NUM_LEDS; i++)
      leds[i] = CHSV(modes[EFF_COLOR].Color[0], modes[EFF_COLOR].Color[1], 255U);
}

// ------------- снегопад ----------
void snowRoutine()
{
  // сдвигаем всё вниз
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    for (uint8_t y = 0U; y < HEIGHT - 1; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1U));
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2U) == 0U && (random(0, 100 - modes[currentMode].Scale) == 0U))
      drawPixelXY(x, HEIGHT - 1U, CHSV(modes[currentMode].Color[0], modes[currentMode].Color[1], 255U));
    else
      drawPixelXY(x, HEIGHT - 1U, 0x000000);
  }
}

// ------------- метель -------------
#define SNOW_DENSE            (60U)                         // плотность снега
#define SNOW_TAIL_STEP        (100U)                        // длина хвоста
void snowStormRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
  }
 
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, 77) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(modes[currentMode].Color[0],  modes[currentMode].Color[1], 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
    drawPixelXY(0, y, getPixColorXY(WIDTH - 1U, y + 1U));
  }

  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    fadePixel(i, HEIGHT - 1U, SNOW_TAIL_STEP);
  }
}
// ------------- матрица ---------------
void matrixRoutine() {
  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1);
    if (thisColor == 0)
      drawPixelXY(x, HEIGHT - 1, 0x00FF00 * (random(0, 15) == 0));
    else if (thisColor < 0x002000)
      drawPixelXY(x, HEIGHT - 1, 0);
    else
      drawPixelXY(x, HEIGHT - 1, thisColor - 0x002000);
  }

  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }
}

// ------------- светлячки --------------
#define LIGHTERS_AM           (100U)
int32_t lightersPos[2U][LIGHTERS_AM];
int8_t lightersSpeed[2U][LIGHTERS_AM];
CHSV lightersColor[LIGHTERS_AM];
uint8_t loopCounter;
int32_t angle[LIGHTERS_AM];
int32_t speedV[LIGHTERS_AM];
int8_t angleSpeed[LIGHTERS_AM];
void lightersRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    randomSeed(millis());
    for (uint8_t i = 0U; i < LIGHTERS_AM; i++)
    {
      lightersPos[0U][i] = random(0, WIDTH * 10);
      lightersPos[1U][i] = random(0, HEIGHT * 10);
      lightersSpeed[0U][i] = random(-10, 10);
      lightersSpeed[1U][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0U, 255U), 255U, 255U);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20U) loopCounter = 0U;
  for (uint8_t i = 0U; i < 70; i++)
  {
    if (loopCounter == 0U)                                  // меняем скорость каждые 255 отрисовок
    {
      lightersSpeed[0U][i] += random(-3, 4);
      lightersSpeed[1U][i] += random(-3, 4);
      lightersSpeed[0U][i] = constrain(lightersSpeed[0U][i], -20, 20);
      lightersSpeed[1U][i] = constrain(lightersSpeed[1U][i], -20, 20);
    }

    lightersPos[0U][i] += lightersSpeed[0U][i];
    lightersPos[1U][i] += lightersSpeed[1U][i];

    if (lightersPos[0U][i] < 0) lightersPos[0U][i] = (WIDTH - 1) * 10;
    if (lightersPos[0U][i] >= (int32_t)(WIDTH * 10)) lightersPos[0U][i] = 0;

    if (lightersPos[1U][i] < 0)
    {
      lightersPos[1U][i] = 0;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
    }
    if (lightersPos[1U][i] >= (int32_t)(HEIGHT - 1) * 10)
    {
      lightersPos[1U][i] = (HEIGHT - 1U) * 10;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
    }
    drawPixelXY(lightersPos[0U][i] / 10, lightersPos[1U][i] / 10, lightersColor[i]);
  }
}

// ------------- белый свет (светится горизонтальная полоса по центру лампы; масштаб - высота центральной горизонтальной полосы; скорость - регулировка от холодного к тёплому; яркость - общая яркость) -------------
void whiteColorStripeRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
    delay(1);

    uint8_t centerY = max((uint8_t)round(HEIGHT / 2.0F) - 1, 0);
    uint8_t bottomOffset = (uint8_t)(!(HEIGHT & 1) && (HEIGHT > 1));                      // если высота матрицы чётная, линий с максимальной яркостью две, а линии с минимальной яркостью снизу будут смещены на один ряд
    for (int16_t y = centerY; y >= 0; y--)
    {
      CRGB color = CHSV(
        45U,                                                                              // определяем тон
        map(modes[EFF_WHITE_COLOR].Speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
        y == centerY                                                                                                    // определяем яркость
          ? 255U                                                                                                        // для центральной горизонтальной полосы (или двух) яркость всегда равна 255
          : (modes[EFF_WHITE_COLOR].Scale / 100.0F) > ((centerY + 1.0F) - (y + 1.0F)) / (centerY + 1.0F) ? 255U : 0U);  // для остальных горизонтальных полос яркость равна либо 255, либо 0 в зависимости от масштаба

      for (uint8_t x = 0U; x < WIDTH; x++)
      {
        drawPixelXY(x, y, color);                                                         // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
        drawPixelXY(x, max((uint8_t)(HEIGHT - 1U) - (y + 1U) + bottomOffset, 0U), color); // при нечётной - одна, но дважды
      }
    }
  }
}

// ------------- мигающий цвет (не эффект! используется для отображения краткосрочного предупреждения; блокирующий код!) -------------
#define WARNING_BRIGHTNESS    (10U)                         // яркость вспышки
void showWarning(
  CRGB color,                                               /* цвет вспышки                                                 */
  uint32_t duration,                                        /* продолжительность отображения предупреждения (общее время)   */
  uint16_t blinkHalfPeriod)                                 /* продолжительность одной вспышки в миллисекундах (полупериод) */
{
  uint32_t blinkTimer = millis();
  enum BlinkState { OFF = 0, ON = 1 } blinkState = BlinkState::OFF;
  FastLED.setBrightness(WARNING_BRIGHTNESS);                // установка яркости для предупреждения
  FastLED.clear();
  delay(2);
  FastLED.show();

  for (uint16_t i = 0U; i < NUM_LEDS; i++)                  // установка цвета всех диодов в WARNING_COLOR
  {
    leds[i] = color;
  }

  uint32_t startTime = millis();
  while (millis() - startTime <= (duration + 5))            // блокировка дальнейшего выполнения циклом на время отображения предупреждения
  {
    if (millis() - blinkTimer >= blinkHalfPeriod)           // переключение вспышка/темнота
    {
      blinkTimer = millis();
      blinkState = (BlinkState)!blinkState;
      FastLED.setBrightness(blinkState == BlinkState::OFF ? 0 : WARNING_BRIGHTNESS);
      delay(1);
      FastLED.show();
    }
    delay(50);
  }

  FastLED.clear();
  FastLED.setBrightness(ONflag ? modes[currentMode].Brightness : 0);  // установка яркости, которая была выставлена до вызова предупреждения
  delay(1);
  FastLED.show();
  loadingFlag = true;                                       // принудительное отображение текущего эффекта (того, что был активен перед предупреждением)
}
// --------------------------- шум ----------------------

// далее идут общие процедуры для эффектов от Stefan Petrick, а непосредственно Комета - в самом низу
CRGB ledsbuff[NUM_LEDS];
const uint8_t e_centerX =  (WIDTH / 2) - 1;
const uint8_t e_centerY = (HEIGHT / 2) - 1;
int8_t zD;
int8_t zF;
// The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS 1

uint32_t e_x[NUM_LAYERS];
uint32_t e_y[NUM_LAYERS];
uint32_t e_z[NUM_LAYERS];
uint32_t e_scaleX[NUM_LAYERS];
uint32_t e_scaleY[NUM_LAYERS];

uint8_t noise3d[NUM_LAYERS][WIDTH][HEIGHT];

uint8_t eNs_noisesmooth;
bool eNs_isSetupped;

void eNs_setup() {
  eNs_noisesmooth = 200;
  for (int i = 0; i < NUM_LAYERS; i++) {
    e_x[i] = random16();
    e_y[i] = random16();
    e_z[i] = random16();
    e_scaleX[i] = 6000;
    e_scaleY[i] = 6000;
  }
  eNs_isSetupped = true;
}

void FillNoise(int8_t layer) {
  for (int8_t i = 0; i < WIDTH; i++) {
    int32_t ioffset = e_scaleX[layer] * (i - e_centerX);
    for (int8_t j = 0; j < HEIGHT; j++) {
      int32_t joffset = e_scaleY[layer] * (j - e_centerY);
      int8_t data = inoise16(e_x[layer] + ioffset, e_y[layer] + joffset, e_z[layer]) >> 8;
      int8_t olddata = noise3d[layer][i][j];
      int8_t newdata = scale8( olddata, eNs_noisesmooth ) + scale8( data, 255 - eNs_noisesmooth );
      data = newdata;
      noise3d[layer][i][j] = data;
    }
  }
}

void MoveX(int8_t delta) {
  //CLS2();
  for (int8_t y = 0; y < HEIGHT; y++) {
    for (int8_t x = 0; x < WIDTH - delta; x++) {
      ledsbuff[XY(x, y)] = leds[XY(x + delta, y)];
    }
    for (int8_t x = WIDTH - delta; x < WIDTH; x++) {
      ledsbuff[XY(x, y)] = leds[XY(x + delta - WIDTH, y)];
    }
  }
  //CLS();
  // write back to leds
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = ledsbuff[XY(x, y)];
    }
  }
}

void MoveY(int8_t delta) {
  //CLS2();
  for (int8_t x = 0; x < WIDTH; x++) {
    for (int8_t y = 0; y < HEIGHT - delta; y++) {
      ledsbuff[XY(x, y)] = leds[XY(x, y + delta)];
    }
    for (int8_t y = HEIGHT - delta; y < HEIGHT; y++) {
      ledsbuff[XY(x, y)] = leds[XY(x, y + delta - HEIGHT)];
    }
  }
  //CLS();
  // write back to leds
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = ledsbuff[XY(x, y)];
    }
  }
}

void MoveFractionalNoiseX(int8_t amplitude = 1, float shift = 0) {
  for (int8_t y = 0; y < HEIGHT; y++) {
    int16_t amount = ((int16_t)noise3d[0][0][y] - 128) * 2 * amplitude + shift * 256  ;
    int8_t delta = abs(amount) >> 8 ;
    int8_t fraction = abs(amount) & 255;
    for (int8_t x = 0 ; x < WIDTH; x++) {
      if (amount < 0) {
        zD = x - delta; zF = zD - 1;
      } else {
        zD = x + delta; zF = zD + 1;
      }
      CRGB PixelA = CRGB::Black  ;
      if ((zD >= 0) && (zD < WIDTH)) PixelA = leds[XY(zD, y)];
      CRGB PixelB = CRGB::Black ;
      if ((zF >= 0) && (zF < WIDTH)) PixelB = leds[XY(zF, y)];
      ledsbuff[XY(x, y)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));   // lerp8by8(PixelA, PixelB, fraction );
    }
  }
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
}

void MoveFractionalNoiseY(int8_t amplitude = 1, float shift = 0) {
  for (int8_t x = 0; x < WIDTH; x++) {
    int16_t amount = ((int16_t)noise3d[0][x][0] - 128) * 2 * amplitude + shift * 256 ;
    int8_t delta = abs(amount) >> 8 ;
    int8_t fraction = abs(amount) & 255;
    for (int8_t y = 0 ; y < HEIGHT; y++) {
      if (amount < 0) {
        zD = y - delta; zF = zD - 1;
      } else {
        zD = y + delta; zF = zD + 1;
      }
      CRGB PixelA = CRGB::Black ;
      if ((zD >= 0) && (zD < HEIGHT)) PixelA = leds[XY(x, zD)];
      CRGB PixelB = CRGB::Black ;
      if ((zF >= 0) && (zF < HEIGHT)) PixelB = leds[XY(x, zF)];
      ledsbuff[XY(x, y)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));
    }
  }
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
}
// NoiseSmearing(by StefanPetrick) Effect mod for GyverLamp by PalPalych 
void MultipleStream() { // 2 comets
  dimAll(192); // < -- затухание эффекта для последующего кадрв

  // gelb im Kreis
  byte xx = 2 + sin8( millis() / 10) / 22;
  byte yy = 2 + cos8( millis() / 10) / 22;
  leds[getPixelNumber( xx, yy)] = 0xFFFF00;

  // rot in einer Acht
  xx = 4 + sin8( millis() / 46) / 32;
  yy = 4 + cos8( millis() / 15) / 32;
  leds[getPixelNumber( xx, yy)] = 0xFF0000;

  // Noise
  e_x[0] += 3000;
  e_y[0] += 3000;
  e_z[0] += 3000;
  e_scaleX[0] = 8000;
  e_scaleY[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(3, 0.33);
  MoveFractionalNoiseY(3);
}

void MultipleStream2() { // 3 comets
  dimAll(220); // < -- затухание эффекта для последующего кадрв
  byte xx = 2 + sin8( millis() / 10) / 22;
  byte yy = 2 + cos8( millis() / 9) / 22;
  leds[getPixelNumber( xx, yy)] += 0x0000FF;

  xx = 4 + sin8( millis() / 10) / 32;
  yy = 4 + cos8( millis() / 7) / 32;
  leds[getPixelNumber( xx, yy)] += 0xFF0000;
  leds[getPixelNumber( e_centerX, e_centerY)] += 0xFFFF00;

  e_x[0] += 3000;
  e_y[0] += 3000;
  e_z[0] += 3000;
  e_scaleX[0] = 8000;
  e_scaleY[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(2);
  MoveFractionalNoiseY(2, 0.33);
}

void MultipleStream3() { // Fireline
  dimAll(160); // < -- затухание эффекта для последующего кадрв
  for (uint8_t i = 1; i < WIDTH; i += 3) {
    leds[getPixelNumber( i, e_centerY)] += CHSV(i * 2 , 255, 255);
  }
  // Noise
  e_x[0] += 3000;
  e_y[0] += 3000;
  e_z[0] += 3000;
  e_scaleX[0] = 8000;
  e_scaleY[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseY(3);
  MoveFractionalNoiseX(3);
}

void MultipleStream4() { // Comet
  dimAll(184); // < -- затухание эффекта для последующего кадрв
  CRGB _eNs_color = CHSV(millis(), 255, 255);
  leds[getPixelNumber( e_centerX, e_centerY)] += _eNs_color;
  // Noise
  e_x[0] += 2000;
  e_y[0] += 2000;
  e_z[0] += 2000;
  e_scaleX[0] = 4000;
  e_scaleY[0] = 4000;
  FillNoise(0);
  MoveFractionalNoiseX(6);
  MoveFractionalNoiseY(5, -0.5);
}

void MultipleStream5() { // Fractorial Fire
  dimAll(140); // < -- затухание эффекта для последующего кадрв
  for (uint8_t i = 1; i < WIDTH; i += 2) {
    leds[XY( i, WIDTH - 1)] += CHSV(i * 2, 255, 255);
  }
  // Noise
  e_x[0] += 3000;
  e_y[0] += 3000;
  e_z[0] += 3000;
  e_scaleX[0] = 8000;
  e_scaleY[0] = 8000;
  FillNoise(0);
  //MoveX(1);
  //MoveY(1);
  MoveFractionalNoiseY(3, 1);
  MoveFractionalNoiseX(2);
}
uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;
  if (y & 0x01)
  {
    // Odd rows run backwards
    uint8_t reverseX = (WIDTH - 1) - x;
    i = (y * WIDTH) + reverseX;
  }
  else
  {
    // Even rows run forwards
    i = (y * WIDTH) + x;
  }
  return i % (WIDTH * HEIGHT + 1);
}
void MultipleStream8() { // Windows ))
  dimAll(96); // < -- затухание эффекта для последующего кадрв на 96/255*100=37%
  for (uint8_t y = 2; y < HEIGHT; y += 5) {
    for (uint8_t x = 2; x < WIDTH; x += 5) {
      leds[getPixelNumber(x, y)]  += CHSV(x * y , 255, 255);
      leds[getPixelNumber(x + 1, y)] += CHSV((x + 4) * y, 255, 255);
      leds[getPixelNumber(x, y + 1)] += CHSV(x * (y + 4), 255, 255);
      leds[getPixelNumber(x + 1, y + 1)] += CHSV((x + 4) * (y + 4), 255, 255);
    }
  }
  // Noise
  e_x[0] += 3000;
  e_y[0] += 3000;
  e_z[0] += 3000;
  e_scaleX[0] = 8000;
  e_scaleY[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(3);
  MoveFractionalNoiseY(3);
}
// прописывается, если ранее нигде не была объявлена (это часто используемая функция у эффектов Stefan Petrick)
void dimAll(uint8_t value) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value); //fadeToBlackBy
  }
}
//  Follow the Rainbow Comet by Palpalych (Effect for GyverLamp 02/03/2020) //
void RainbowComet(uint8_t scale = 4) { // Rainbow Comet by PalPalych
  dimAll(254U); // < -- затухание эффекта для последующего кадра
  CRGB _eNs_color = CRGB::White;
  if (scale < 50) {
    _eNs_color = CHSV(millis() / scale * 4, 255, 255);
  } else if (scale < 100) {
    _eNs_color = CHSV((scale - 50) * 5, 255, 255);
  }
  leds[getPixelNumber(e_centerX, e_centerY)] += _eNs_color;
  leds[getPixelNumber(e_centerX + 1, e_centerY)] += _eNs_color;
  leds[getPixelNumber(e_centerX, e_centerY + 1)] += _eNs_color;
  leds[getPixelNumber(e_centerX + 1, e_centerY + 1)] += _eNs_color;

  // Noise
  e_x[0] += 1500;
  e_y[0] += 1500;
  e_z[0] += 1500;
  e_scaleX[0] = 8000;
  e_scaleY[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(WIDTH / 2U - 1U);
  MoveFractionalNoiseY(HEIGHT / 2U - 1U);
}
void RainbowCometRoutine() {     
  RainbowComet(map(modes[currentMode].Scale, 0U, 255U, 10U, 255U));    // <--- вызов эффекта с параметрами Rainbow Comet by PalPalych
}
