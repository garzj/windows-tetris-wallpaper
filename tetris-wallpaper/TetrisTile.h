#pragma once

#include <SDL.h>
#include <vector>
#include "IconGrid.h"
#include "sdlext.h"

class TetrisTile {
public:
  POINT pos;
  std::vector<POINT> blocks;

  static std::shared_ptr<TetrisTile> MakeRandomTile(SDL_Renderer* renderer, IconGrid* iconGrid, POINT pos);

  TetrisTile(SDL_Renderer* renderer, IconGrid* iconGrid, POINT pos, std::vector<POINT> blocks);

  void Render(bool iconVisible=true);

  bool Collides(std::shared_ptr<TetrisTile> other);
  bool Collides(POINT block);

  void Rotate(bool left);

  void Translate(POINT tr);

private:
  SDL_Renderer* renderer;
  IconGrid* iconGrid;
  SDL_Color color;
};
