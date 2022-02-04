#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <regex>
#include <sstream>
#include "util.h"
#include "winext.h"

class IconGrid;

class Icon {
public:
  IconGrid* grid;
  LPITEMIDLIST pidl;
  bool iconVisible = true;

  Icon(IconGrid* grid, LPITEMIDLIST pidl) : grid(grid), pidl(pidl) {};
  ~Icon();

  void GetPosition(POINT& pt);
  void SetPosition(POINT& pt);
};

class UserIcon {
public:
  std::shared_ptr<Icon> icon;

  UserIcon(std::shared_ptr<Icon> icon);
  ~UserIcon();

private:
  POINT userPt;
};

class ProgramIcon {
public:
  IconGrid* grid;
  std::shared_ptr<Icon> icon;

  ProgramIcon(IconGrid* grid);
  ~ProgramIcon();

  void AwaitInit();
};

class IconGrid {
public:
  MonitorInfo monitor;
  SIZE size;
  CComPtr<IFolderView2> spView;
  CComPtr<IShellFolder> spFolder;

  IconGrid(MonitorInfo monitor, SIZE size);
  ~IconGrid();

  void SetVisible(POINT pos, bool visible);
  void GetRect(POINT pos, RECT& rect);

private:
  DWORD userFlags;
  std::vector<std::shared_ptr<UserIcon>> userIcons;
  std::vector<std::shared_ptr<ProgramIcon>> programIcons;
  std::vector<std::shared_ptr<Icon>> grid;

  void LoadIcons(std::vector<std::shared_ptr<Icon>>& icons);
};
