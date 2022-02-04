#include "TetrisWallpaper.h"

TetrisWallpaper::TetrisWallpaper(IconGrid* iconGrid) : iconGrid(iconGrid) {
  window = SDL_CreateWindow(
    "Live Wallpaper",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    100,
    100,
    SDL_WINDOW_BORDERLESS);
  if (window == NULL) {
    std::cout << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    // TODO: Throw
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    // TODO: Throw
    return;
  }

  // Get HWND from SDL
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  hwnd = wmInfo.info.win.window;

  // Mask black (the background) to be transparent
  SetWindowLong(
    hwnd,
    GWL_EXSTYLE,
    GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

  // Attach as child of desktop
  if (!bAttached) {
    SetDesktopChildWindow(hwnd);
    bAttached = true;
  }

  // Resize and move window
  windowRect = iconGrid->monitor.rect;
  SIZE winSize = {
    windowRect.right - windowRect.left,
    windowRect.bottom - windowRect.top
  };
  SDL_SetWindowSize(window, winSize.cx, winSize.cy);
  MoveWindow(hwnd, 0, 0, winSize.cx, winSize.cy, TRUE);

  // Init bounding blocks
  for (long x = -1, y = -1; x <= iconGrid->size.cx; x++)
    boundingBlocks.push_back({ x, y });
  for (long x = -1, y = iconGrid->size.cy; x <= iconGrid->size.cx; x++)
    boundingBlocks.push_back({ x, y });
  for (long x = -1, y = 0; y < iconGrid->size.cy; y++)
    boundingBlocks.push_back({ x, y });
  for (long x = iconGrid->size.cx, y = 0; y < iconGrid->size.cy; y++)
    boundingBlocks.push_back({ x, y });

  bRunning = true;

  // Start the game with a new tile
  NewTile();
}

TetrisWallpaper::~TetrisWallpaper() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

void TetrisWallpaper::Frame() {
  // Events
  SDL_Event e;
  SDL_PollEvent(&e);
  if (e.type == SDL_QUIT) {
    bRunning = false;
  }

  if (GetAsyncKeyState(VK_F8) & 0x01) {
    bRunning = false;
  }

  // Movement (key move, rotate, auto move)
  uint64_t curTime = GetTimeInMS();
  POINT move = { 0l, 0l };
  bool rotate = false;

  if (GetAsyncKeyState(VK_LEFT) & 0x01) {
    move.x -= 1l;
  }
  if (GetAsyncKeyState(VK_RIGHT) & 0x01) {
    move.x += 1l;
  }
  if (GetAsyncKeyState(VK_UP) & 0x01) {
    rotate = true;
  }
  if (GetAsyncKeyState(VK_DOWN) & 0x01) {
    move.y += 1l;
    lastMove = curTime;
  }
  if (curTime - lastMove >= 750) {
    move.y += 1l;
    lastMove = curTime;
  }

  if (rotate) {
    curTile->Render(false);
    curTile->Rotate(true);
    if (CollidesAny(curTile)) {
      curTile->Rotate(false);
    }
    curTile->Render(true);
  }

  for (int x = 0; x < std::abs(move.x); x++) {
    long trX = move.x < 0 ? -1l : 1l;
    curTile->Render(false);
    curTile->Translate({ trX, 0l });
    if (CollidesAny(curTile)) {
      curTile->Translate({ -trX, 0l });
    }
    curTile->Render(true);
  }

  for (int y = 0; y < move.y; y++) {
    long trY = 1;
    curTile->Render(false);
    curTile->Translate({ 0l, trY });
    if (CollidesAny(curTile)) {
      curTile->Translate({ 0l, -trY });

      // Tile landed!
      curTile->Render(true);
      landedTiles.push_back(curTile);

      // Check for full rows
      for (long y = 0; y < iconGrid->size.cy; y++) {
        bool bFull = true;
        for (long x = 0; x < iconGrid->size.cx; x++) {
          bool bFound = false;
          for (auto& tile : landedTiles) {
            if (bFound) break;
            for (auto& block : tile->blocks) {
              if (tile->pos.x + block.x == x && tile->pos.y + block.y == y) {
                bFound = true;
                break;
              }
            }
          }
          if (!bFound) {
            bFull = false;
            break;
          }
        }

        if (bFull) {
          // Kill line
          std::cout << "Killed a line with " << iconGrid->size.cx << " blocks!\n";

          for (long x = 0; x < iconGrid->size.cx; x++) {
            for (size_t tileIndex = landedTiles.size() - 1; tileIndex >= 0; tileIndex--) {
              auto& tile = landedTiles.at(tileIndex);
              bool deleted = false;
              for (size_t i = tile->blocks.size() - 1; i >= 0; i--) {
                if (tile->pos.y + tile->blocks[i].y == y) {
                  if (!deleted) {
                    deleted = true;
                    tile->Render(false);
                  }
                  tile->blocks.erase(tile->blocks.begin() + i);
                }

                if (tile->blocks.size() == 0) {
                  landedTiles.erase(landedTiles.begin() + tileIndex);
                }
                
                // TODO: Copy tile, split it
                //auto& newTile = std::make_shared<TetrisTile>(tile);
                //landedTiles.push_back(newTile);
              }
              if (deleted)
                tile->Render(true);
            }
          }

          // Let lines fall
          for (auto& tile : landedTiles) {
            tile->Render(false);
          }
          for (auto& tile : landedTiles) {
            bool bFell;
            do {
              bFell = RecursiveFall(tile);
            } while (bFell);
          }
          for (auto& tile : landedTiles) {
            tile->Render(true);
          }
        }
      }

      NewTile();

      break;
    }
    curTile->Render(true);
  }

  // Render
  if (bFirstFrame) {
    ClearWindow();
    SDL_RenderPresent(renderer);
    bFirstFrame = false;
  }

  SDL_RenderPresent(renderer);
}

bool TetrisWallpaper::CollidesAny(std::shared_ptr<TetrisTile> tile) {
  for (auto& landedTile : landedTiles) {
    if (landedTile != tile && tile->Collides(landedTile))
      return true;
  }
  for (auto& block : boundingBlocks) {
    if (tile->Collides(block))
      return true;
  }
  return false;
}

bool TetrisWallpaper::RecursiveFall(std::shared_ptr<TetrisTile> tile) {
  std::set<TetrisTile*> queue;
  return RecursiveFall(tile, queue);
}

bool TetrisWallpaper::RecursiveFall(std::shared_ptr<TetrisTile> tile, std::set<TetrisTile*>& queue) {
  tile->Render(false);
  tile->Translate({ 0l, 1l });

  for (auto& block : boundingBlocks) {
    if (tile->Collides(block)) {
      tile->Translate({ 0l, -1l });
      return false;
    }
  }

  for (auto& landedTile : landedTiles) {
    if (landedTile == tile) continue;

    // Ignore all tiles that depend on the current tile to avoid loops
    if (queue.find(landedTile.get()) != queue.end()) continue;

    if (tile->Collides(landedTile)) {
      queue.insert(tile.get());
      bool fell = RecursiveFall(landedTile, queue);
      queue.erase(tile.get());

      if (!fell) {
        tile->Translate({ 0l, -1l });
      }
      return fell;
    }
  }

  return true;
}

void TetrisWallpaper::NewTile() {
  curTile = TetrisTile::MakeRandomTile(renderer, iconGrid, {
    iconGrid->size.cx / 2,
    2
    });
  if (CollidesAny(curTile)) {
    std::cout << "You lost :/\n";
    bRunning = false;
  }

  curTile->Render(true);
}

void TetrisWallpaper::ClearWindow() {
  // Background
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);
}
