#pragma once

#include "IconGrid.h"

// Icon
Icon::~Icon() {
  CoTaskMemFree(pidl);
}

void Icon::GetPosition(POINT& pt) {
  ThrowIfFailed(grid->spView->GetItemPosition(pidl, &pt), "Failed to retrieve item position.");
}

void Icon::SetPosition(POINT& pt) {
  PCITEMID_CHILD apidl[1] = { pidl };
  ThrowIfFailed(grid->spView->SelectAndPositionItems(1, apidl, &pt, SVSI_POSITIONITEM), "Failed to modify item position.");
}

// UserIcon: Saves and restores an icon position on con- / destruction
UserIcon::UserIcon(std::shared_ptr<Icon> icon) : icon(icon) {
  icon->GetPosition(userPt);
}

UserIcon::~UserIcon() {
  icon->SetPosition(userPt);
}

// ProgramIcon: Creates and destroys a random program icon on con- / destruction
ProgramIcon::ProgramIcon(IconGrid *grid) : grid(grid) {
  // Get start menu programs folder
  CComPtr<IShellFolder> desktopFolder;
  ThrowIfFailed(SHGetDesktopFolder(&desktopFolder), "Failed to retrieve the desktop folder.");
  CComHeapPtr<ITEMIDLIST> smpFolderId;
  ThrowIfFailed(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &smpFolderId), "Failed to retrieve the start menu programs folder id.");
  CComPtr<IShellFolder> smpFolder;
  ThrowIfFailed(desktopFolder->BindToObject(smpFolderId, NULL, IID_PPV_ARGS(&smpFolder)), "Failed to retrieve the start menu programs folder interface.");

  // Get grid folder
  CComHeapPtr<ITEMIDLIST> folderId;
  ThrowIfFailed(SHGetIDListFromObject(grid->spFolder, &folderId), "Failed to retrieve the grid folder id.");
  CComPtr<IShellItem> folderItem;
  SHCreateItemFromIDList(folderId, IID_PPV_ARGS(&folderItem));

  // Get enum
  CComPtr<IEnumIDList> spEnum;
  smpFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS, &spEnum);

  // Count items
  CComHeapPtr<ITEMIDLIST> smpIconId;
  int itemCount;
  spEnum->Reset();
  for (itemCount = 0; spEnum->Next(1, &smpIconId, NULL) == S_OK; itemCount++) {
    smpIconId.Free();
  }
  if (itemCount == 0) {
    throw std::system_error{ 2, std::system_category(), "Could not find any items in the start menu programs folder." };
    icon = NULL;
    return;
  }

  // Pick random item
  int randItem = rand() % itemCount;
  spEnum->Reset();
  for (int i = 0; i < randItem && spEnum->Next(1, &smpIconId, NULL) == S_OK; i++) {
    smpIconId.Free();
  }
  ThrowIfFailed(spEnum->Next(1, &smpIconId, NULL), "Failed to pick a random icon.");

  CComPtr<IShellItem> smpIcon;
  // Using SHCreateItemFromIDList still results in an item with the desktop path.. why..
  //SHCreateItemFromIDList(smpIconId, IID_PPV_ARGS(&smpIcon));
  SHCreateItemWithParent(smpFolderId, NULL, smpIconId, IID_PPV_ARGS(&smpIcon));

  // Create grid icon
  CComPtr<IShellItem> copyItem;
  SHCreateItemWithParent(folderId, NULL, smpIconId, IID_PPV_ARGS(&copyItem));
  CComHeapPtr<WCHAR> copyName;
  ThrowIfFailed(copyItem->GetDisplayName(SIGDN_FILESYSPATH, &copyName), "Failed to get new path of the random icon.");

  // Copy the item to the desktop
  CComPtr<IFileOperation> spfo;
  CreateFileOperation(IID_PPV_ARGS(&spfo));
  
  std::wstring wsCopyName(copyName);
  std::wregex reg(L".lnk");
  std::wostringstream wossSuf;
  wossSuf << L"-" << std::to_wstring(rand()) << L".lnk";
  std::wstring str = std::regex_replace(wsCopyName, reg, wossSuf.str());
  StrCpyW(copyName, str.c_str());
  std::wcout << static_cast<std::wstring>(copyName) << "\n";

  spfo->CopyItem(smpIcon, folderItem, copyName, NULL);

  ThrowIfFailed(spfo->PerformOperations(), "Failed to copy the random icon.");

  // Retrieve the new pidl
  LPITEMIDLIST pidl = NULL;
  //ThrowIfFailed(spFolder->ParseDisplayName(NULL, NULL, copyName, NULL, &pidl, NULL), "Failed to get new pidl of the random icon.");
  // Somehow the id sometimes doesn't work with the shell view, so get it from that folder by path..
  spEnum.Release();
  grid->spFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS, &spEnum);
  for (LPITEMIDLIST itemId; spEnum->Next(1, &itemId, NULL) == S_OK; ) {
    STRRET str;
    ThrowIfFailed(grid->spFolder->GetDisplayNameOf(itemId, SHGDN_FORPARSING, &str), "Failed to get the display name of the random icon.");
    CComHeapPtr<WCHAR> itemName;
    StrRetToStr(&str, itemId, &itemName);

    if (StrCmpW(copyName, itemName) == 0) {
      if (pidl != NULL) {
        CoTaskMemFree(pidl);
      }
      pidl = itemId;
      break;
    }

    CoTaskMemFree(itemId);
  }

  icon = std::make_shared<Icon>(grid, pidl);
}

// Waits, til the icon can be used
void ProgramIcon::AwaitInit() {
  POINT pt;
  while (FAILED(icon->grid->spView->GetItemPosition(icon->pidl, &pt))) {
    Sleep(100);
  }
}

ProgramIcon::~ProgramIcon() {
  // Get item handle
  CComPtr<IShellItem> item;
  SHCreateItemFromIDList(icon->pidl, IID_PPV_ARGS(&item));

  // Delete item
  CComPtr<IFileOperation> spfo;
  CreateFileOperation(IID_PPV_ARGS(&spfo));
  spfo->DeleteItem(item, NULL);
  ThrowIfFailed(spfo->PerformOperations(), "Failed to delete a program icon.");
}

// IconGrid
IconGrid::IconGrid(MonitorInfo monitor, SIZE size) : monitor(monitor), size(size) {
  FindDesktopFolderView(IID_PPV_ARGS(&spView));
  spView->GetFolder(IID_PPV_ARGS(&spFolder));

  // Save user flags
  spView->GetCurrentFolderFlags(&userFlags);

  // Remove snap to grid
  DWORD progFlags = userFlags & ~FWF_SNAPTOGRID;
  spView->SetCurrentFolderFlags(~0, progFlags);

  // Add existing user icons
  std::vector<std::shared_ptr<Icon>> icons;
  LoadIcons(icons);
  for (auto& icon : icons) {
    userIcons.push_back(std::make_shared<UserIcon>(icon));
    grid.push_back(icon);
  }
  icons.clear();

  // Fill with new program icons
  while (grid.size() < size.cx * size.cy) {
    auto programIcon = std::make_shared<ProgramIcon>(this);
    programIcons.push_back(programIcon);
    grid.push_back(programIcon->icon);
  }
  for (auto& programIcon : programIcons) {
    programIcon->AwaitInit();
  }

  // Hide items by default
  for (long x = 0; x < size.cx; x++) {
    for (long y = 0; y < size.cy; y++) {
      SetVisible({ x, y }, false);
    }
  }
}

IconGrid::~IconGrid() {
  grid.clear();

  // Destroy program icons
  programIcons.clear();

  // Restore previously existing user icons
  userIcons.clear();

  // Restore user flags
  spView->SetCurrentFolderFlags(~0, userFlags);
}

void IconGrid::SetVisible(POINT pos, bool visible) {
  if (pos.x < 0 || pos.x >= size.cx) return;
  if (pos.y < 0 || pos.y >= size.cy) return;
  int iconIndex = pos.x * size.cy + pos.y;

  if (grid[iconIndex]->iconVisible == visible) return;
  grid[iconIndex]->iconVisible = visible;

  // Position under the screen, if not visible
  if (!visible) {
    pos.y += size.cy;
  }

  RECT rect;
  GetRect(pos, rect);

  POINT pt = { rect.left + monitor.rect.left, rect.top + monitor.rect.top };
  grid[iconIndex]->SetPosition(pt);
}

void IconGrid::GetRect(POINT pos, RECT& rect) {
  SIZE monSize = {
    monitor.rect.right - monitor.rect.left,
    monitor.rect.bottom - monitor.rect.top
  };

  long blockSize = monSize.cy / size.cy;

  SIZE fieldSize = {
    blockSize * size.cx,
    blockSize * size.cy
  };

  rect.left = (monSize.cx / 2 - fieldSize.cx / 2) + blockSize * pos.x;
  rect.right = rect.left + blockSize - 1;
  rect.top = blockSize * pos.y;
  rect.bottom = rect.top + blockSize - 1;
}

void IconGrid::LoadIcons(std::vector<std::shared_ptr<Icon>>& icons) {
  CComPtr<IEnumIDList> spEnum;
  spView->Items(SVGIO_ALLVIEW, IID_PPV_ARGS(&spEnum));
  while (true) {
    LPITEMIDLIST pidl;
    if (spEnum->Next(1, &pidl, NULL) != S_OK) break;

    icons.push_back(std::make_shared<Icon>(this, pidl));
  }
}
