#include "sdlext.h"
#include <SDL.h>

void FPSLimiter::Tick(int limit) {
  int endTick = SDL_GetTicks();
  if (startTick == 0) {
    startTick = endTick;
  }
  int Dt = endTick - startTick;
  if ((1000 / limit) > Dt) SDL_Delay(1000 / limit - (Dt));

  startTick = SDL_GetTicks();
}

SDL_Color HsvToRgb(uint8_t hsvH, uint8_t hsvS, uint8_t hsvV, uint8_t a) {
  SDL_Color rgb;
  unsigned char region, remainder, p, q, t;

  rgb.a = a;

  if (hsvS == 0) {
    rgb.r = hsvV;
    rgb.g = hsvV;
    rgb.b = hsvV;
    return rgb;
  }

  region = hsvH / 43;
  remainder = (hsvH - (region * 43)) * 6;

  p = (hsvV * (255 - hsvS)) >> 8;
  q = (hsvV * (255 - ((hsvS * remainder) >> 8))) >> 8;
  t = (hsvV * (255 - ((hsvS * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
  case 0:
    rgb.r = hsvV;
    rgb.g = t;
    rgb.b = p;
    break;
  case 1:
    rgb.r = q;
    rgb.g = hsvV;
    rgb.b = p;
    break;
  case 2:
    rgb.r = p;
    rgb.g = hsvV;
    rgb.b = t;
    break;
  case 3:
    rgb.r = p;
    rgb.g = q;
    rgb.b = hsvV;
    break;
  case 4:
    rgb.r = t;
    rgb.g = p;
    rgb.b = hsvV;
    break;
  default:
    rgb.r = hsvV;
    rgb.g = p;
    rgb.b = q;
    break;
  }

  return rgb;
}
