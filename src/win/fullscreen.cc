#include "../fullscreen.h"

#include <Windows.h>

// NOTES:
//
// https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-styles
// https://chromium.googlesource.com/chromium/src/+/3710d00089e1b6d64a92a50a060b9581abd33fcd/chrome/browser/fullscreen_win.cc
// http://www.jasinskionline.com/windowsapi/ref/g/getwindowlong.html
// WS_MAXIMIZEBOX?
// https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-styles 


/**
 * this code is lightly modified from the chromium source code at:
 * https://chromium.googlesource.com/chromium/src/+/3710d00089e1b6d64a92a50a060b9581abd33fcd/chrome/browser/fullscreen_win.cc
 */
bool isFullscreenMode() {
  // Get the foreground window which the user is currently working on.
  HWND wnd = ::GetForegroundWindow();
  if (!wnd) {
    return false;
  }
  // Get the monitor where the window is located.
  RECT wnd_rect;
  if (!::GetWindowRect(wnd, &wnd_rect)) {
    return false;
  }
  HMONITOR monitor = ::MonitorFromRect(&wnd_rect, MONITOR_DEFAULTTONULL);
  if (!monitor) {
    return false;
  }

  MONITORINFO monitor_info = { sizeof(monitor_info) };
  if (!::GetMonitorInfo(monitor, &monitor_info)) {
    return false;
  }

  // The window should be at least as large as the monitor.
  if (!::IntersectRect(&wnd_rect, &wnd_rect, &monitor_info.rcMonitor)) {
    return false;
  }

  if (!::EqualRect(&wnd_rect, &monitor_info.rcMonitor)) {
    return false;
  }

  // At last, the window style should not have WS_DLGFRAME and WS_THICKFRAME and
  // its extended style should not have WS_EX_WINDOWEDGE and WS_EX_TOOLWINDOW.
  LONG style = ::GetWindowLong(wnd, GWL_STYLE);
  LONG ext_style = ::GetWindowLong(wnd, GWL_EXSTYLE);
  return !((style & (WS_DLGFRAME | WS_THICKFRAME)) ||
           (ext_style & (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW)));
}
