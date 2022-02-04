#pragma once

#include <conio.h>
#include <SDL.h>
#include "winext.h"
#include <SDL_syswm.h>
#include "IconGrid.h"
#include "TetrisTile.h"
#include "sdlext.h"
#include "util.h"
#include <set>

class TetrisWallpaper {
private:
  bool bRunning = false;
  bool bFirstFrame = true;
  bool bAttached = false;
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  HWND hwnd = nullptr;
  RECT windowRect = { 0 };

  IconGrid* iconGrid;
  uint64_t lastMove = GetTimeInMS();
  std::vector<POINT> boundingBlocks;
  std::vector<std::shared_ptr<TetrisTile>> landedTiles;
  std::shared_ptr<TetrisTile> curTile;

  void ClearWindow();

public:
  TetrisWallpaper(IconGrid* iconGrid);
  ~TetrisWallpaper();

  bool IsRunning() {
    return bRunning;
  }

  void Frame();

  bool CollidesAny(std::shared_ptr<TetrisTile> tile);
  bool RecursiveFall(std::shared_ptr<TetrisTile> tile);
  bool RecursiveFall(std::shared_ptr<TetrisTile> tile, std::set<TetrisTile*>& queue);
  void NewTile();
};
