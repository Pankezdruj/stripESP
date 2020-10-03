

void fireRoutine(bool isColored)                            // true - цветной огонь, false - белый
{
    for (int i = 0; i < NUM_LEDS / 2; i++){
      leds[i] = CHSV(map(i, 0, NUM_LEDS / 2, 0, 15), 255, random(101)>map(i, 0, NUM_LEDS / 2, 0, 101) ? map(i, 0, NUM_LEDS / 2, 10, 0) : map(i, 0, NUM_LEDS / 2, 0, BRIGHTNESS));
    }
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) leds[i] = CHSV(map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, 15, 0), 255, random(101)>map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, 101, 0) ? map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, 10, 0) : map(i - NUM_LEDS / 2, 0, NUM_LEDS / 2, BRIGHTNESS, 0));
}
