#pragma once


struct ModeType
{
  uint8_t Brightness = 50U;
  uint8_t BGBrightness = 0U;
  uint8_t Scale = 40U;
  uint8_t Color[2] = {20U, 20U};
};

typedef void (*SendCurrentDelegate)(char *outputBuffer);
typedef void (*ShowWarningDelegate)(CRGB color, uint32_t duration, uint16_t blinkHalfPeriod);
