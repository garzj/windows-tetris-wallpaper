#if not defined(WIN32) && not defined(_WIN32) && not defined(__WIN32)
#error "This project can only be compiled for windows platforms."
#endif

#include <SDL.h>
#include <ctime>
#include <random>
#include <iostream>
#include <conio.h>
#include <signal.h>
#include "TetrisWallpaper.h"
#include "sdlext.h"
#include "util.h"
#include "winext.h"
#include "IconGrid.h"

bool gSigInt = false;

static void onSigInt(int s) {
  gSigInt = true;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, onSigInt);
  signal(SIGBREAK, onSigInt);

  try {
    // Init
    CComInit cComInit;
    srand((unsigned)time(0));

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
      std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError()
        << std::endl;
      return 1;
    }

    // Pick a monitor
    std::vector<MonitorInfo> monitors;
    GetMonitors(monitors);
    if (monitors.size() < 0) {
      std::cout << "What's wrong with you, go buy a monitor :)\n";
      (void)_getch();
      return 1;
    }
    MonitorInfo monitor = monitors[0];

    // Icon grid
    IconGrid* iconGrid = new IconGrid(monitor, { 10l, 20l });

    // Tetris Wallpaper
    FPSLimiter fpsLimiter;

    TetrisWallpaper* tetris = new TetrisWallpaper(iconGrid);
    while (!gSigInt && tetris->IsRunning()) {
      fpsLimiter.Tick(60);

      tetris->Frame();
    }
    delete tetris;

    delete iconGrid;

    SDL_Quit();
  } catch (std::system_error const& e) {
    std::cout << "ERROR: " << e.what() << ", error code: " << e.code() << "\n";
    (void)_getch();
    return 1;
  }

  return 0;
}
