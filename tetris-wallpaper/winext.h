#pragma once

#include <ExDisp.h>
#include <ObjIdl.h>
#include <ShObjIdl_core.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <atlbase.h>
#include <objbase.h>
#include <windows.h>
#include <iostream>
#include <vector>

// Monitors
struct MonitorInfo {
  HMONITOR handle;
  RECT rect;
};

void GetMonitors(std::vector<MonitorInfo>& monitors);

void UnionMonitorsRect(const std::vector<MonitorInfo>& monitors, LPRECT lprc);

// Desktop
void SetDesktopChildWindow(HWND hwnd);

// Shell API
template<typename T> void ThrowIfFailed(HRESULT hr, T&& msg) {
  if (FAILED(hr))
    throw std::system_error{ hr, std::system_category(), std::forward<T>(msg) };
}

class CComInit {
public:
  CComInit();
  ~CComInit();

  CComInit(CComInit const&) = delete;
  CComInit& operator=(CComInit const&) = delete;
};

void CreateFileOperation(REFIID riid, void** ppv);

void FindDesktopFolderView(REFIID riid, void** ppv);

// Debug
std::ostream& operator<< (std::ostream& out, POINT const& pt);
std::ostream& operator<< (std::ostream& out, RECT const& rect);
