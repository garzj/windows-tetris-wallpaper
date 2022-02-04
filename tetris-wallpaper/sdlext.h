#pragma once

#include <SDL.h>

class FPSLimiter {
private:
  int startTick = 0;

public:
  void Tick(int limit);
};

SDL_Color HsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t a = 255);
