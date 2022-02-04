#define _WIN32_WINNT 0x600

#include "winext.h"

// Monitors
void FixHighDPIMonitorRect(HMONITOR hMon, LPRECT lprcMonitor) {
  SetProcessDPIAware();

  MONITORINFOEXA monInfo;
  monInfo.cbSize = sizeof(MONITORINFOEXA);
  GetMonitorInfoA(hMon, &monInfo);

  DISPLAY_DEVICEA displayDev = { 0 };
  displayDev.cb = sizeof(DISPLAY_DEVICEA);
  for (int index = 0;; index++) {
    if (!EnumDisplayDevicesA(
      NULL,
      index,
      &displayDev,
      EDD_GET_DEVICE_INTERFACE_NAME))
      break;
    if (strcmp(monInfo.szDevice, displayDev.DeviceName) != 0) continue;

    DEVMODEA devMode;
    devMode.dmSize = sizeof(DEVMODEA);
    DISPLAY_DEVICEA displayDev;
    displayDev.cb = sizeof(DISPLAY_DEVICEA);
    if (!EnumDisplayDevicesA(
      NULL,
      index,
      &displayDev,
      EDD_GET_DEVICE_INTERFACE_NAME))
      break;
    if (!EnumDisplaySettingsA(
      displayDev.DeviceName,
      ENUM_CURRENT_SETTINGS,
      &devMode))
      break;

    lprcMonitor->bottom = lprcMonitor->top + devMode.dmPelsHeight;
    lprcMonitor->right = lprcMonitor->left + devMode.dmPelsWidth;
    break;
  }
}

static BOOL CALLBACK
ProcMonitor(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM lParam) {
  FixHighDPIMonitorRect(hMon, lprcMonitor);

  std::vector<MonitorInfo>& monitors = *(std::vector<MonitorInfo> *)lParam;
  MonitorInfo monitor;
  monitor.handle = hMon;
  monitor.rect = *lprcMonitor;
  monitors.push_back(monitor);

  return TRUE;
}
void GetMonitors(std::vector<MonitorInfo>& monitors) {
  EnumDisplayMonitors(NULL, NULL, &ProcMonitor, (LPARAM)&monitors);
}

void UnionMonitorsRect(const std::vector<MonitorInfo>& monitors, LPRECT lprc) {
  SetRectEmpty(lprc);
  for (auto& monitor : monitors) {
    UnionRect(lprc, lprc, &monitor.rect);
  }
}

// Desktop
static BOOL CALLBACK ProcDesktopWorker(HWND hwnd, LPARAM lParam) {
  HWND shellDll = FindWindowExA(hwnd, NULL, "SHELLDLL_DefView", NULL);
  if (shellDll != NULL) {
    *(HWND*)lParam = FindWindowExA(NULL, hwnd, "WorkerW", NULL);
    return FALSE;
  }
  return TRUE;
}
void SetDesktopChildWindow(HWND hwnd) {
  int WM_SPAWN_WORKER = 0x052C;

  HWND progman = FindWindowA("ProgMan", NULL);
  LRESULT result = SendMessageTimeoutA(
    progman,
    WM_SPAWN_WORKER,
    0,
    0,
    SMTO_NORMAL,
    1000,
    NULL);
  if (!result) {
    std::string lastError = std::system_category().message(GetLastError());
    std::cout << "Unable to spawn new worker: " + lastError + "\n";
    return;
  }

  HWND worker = NULL;
  EnumWindows(&ProcDesktopWorker, (LPARAM)&worker);
  if (worker == NULL) {
    std::string lastError = std::system_category().message(GetLastError());
    std::cout << "Unable to find WorkerW: " << lastError << "\n";
    return;
  }

  SetParent(hwnd, worker);
}

// Shell API
CComInit::CComInit() {
  ThrowIfFailed(CoInitialize(NULL), "CoInitialize failed");
}

CComInit::~CComInit() {
  CoUninitialize();
}

void FindDesktopFolderView(REFIID riid, void** ppv) {
  CComPtr<IShellWindows> spShellWindows;
  ThrowIfFailed(
    spShellWindows.CoCreateInstance(CLSID_ShellWindows),
    "Failed to create IShellWindows instance.");

  CComVariant vtLoc(CSIDL_DESKTOP);
  CComVariant vtEmpty;
  long lhwnd;
  CComPtr<IDispatch> spdisp;
  ThrowIfFailed(
    spShellWindows->FindWindowSW(
      &vtLoc,
      &vtEmpty,
      SWC_DESKTOP,
      &lhwnd,
      SWFO_NEEDDISPATCH,
      &spdisp),
    "Failed to find desktop window."
  );

  CComQIPtr<IServiceProvider> spProv(spdisp);
  if (!spProv)
    ThrowIfFailed(E_NOINTERFACE, "Failed to get IServiceProvider interface for desktop.");

  CComPtr<IShellBrowser> spBrowser;
  ThrowIfFailed(
    spProv->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&spBrowser)),
    "Failed to get IShellBrowser for desktop.");

  CComPtr<IShellView> spView;
  ThrowIfFailed(
    spBrowser->QueryActiveShellView(&spView),
    "Failed to query IShellView for desktop.");

  ThrowIfFailed(
    spView->QueryInterface(riid, ppv),
    "Could not query desktop IShellView.");
}

void CreateFileOperation(REFIID riid, void** ppv) {
  *ppv = NULL;
  CComPtr<IFileOperation> spfo;
  ThrowIfFailed(CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_ALL, IID_PPV_ARGS(&spfo)), "Could not create a file operation interface.");
  ThrowIfFailed(spfo->SetOperationFlags(FOF_NO_UI), "Could not set file operation flags.");
  spfo->QueryInterface(riid, ppv);
}

// Debug
std::ostream& operator<< (std::ostream& out, POINT const& pt) {
  out << "PT(" << pt.x << ", " << pt.y << ")";
  return out;
}
std::ostream& operator<< (std::ostream& out, RECT const& rect) {
  out << "RECT(" << rect.left << ", " << rect.top << ")(" << rect.right << ", " << rect.bottom << ")";
  return out;
}
