#include <nan.h>
#include <iostream>

#ifdef IS_MAC
#include <Carbon/Carbon.h>
#include <AppKit/AppKit.h>
#define NOT_FULLSCREEN_KEY "Menubar"
#endif

#ifdef IS_WINDOWS
#include <Windows.h>
#endif

NAN_METHOD(isFullscreen);

// Example with node ObjectWrap
// Based on https://nodejs.org/api/addons.html#addons_wrapping_c_objects but using NAN
class Fullscreen : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

  private:
    explicit Fullscreen();
    ~Fullscreen();

    static NAN_METHOD(New);
    static Nan::Persistent<v8::Function> constructor;
};

Nan::Persistent<v8::Function> Fullscreen::constructor;

NAN_MODULE_INIT(Fullscreen::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Fullscreen").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("Fullscreen").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

//Fullscreen::Fullscreen(double value) : value_(value) {
Fullscreen::Fullscreen() {
}

Fullscreen::~Fullscreen() {
}

NAN_METHOD(Fullscreen::New) {
  if (info.IsConstructCall()) {
    Fullscreen *obj = new Fullscreen();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } 
  else {
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
  }
}

using v8::FunctionTemplate;

#ifdef IS_WINDOWS
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
bool IsFullScreenWindowMode() {
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
#endif

//bool Method(napi_env env, napi_callback_info info) {
NAN_METHOD(isFullscreen) {
  #ifdef IS_MAC

  //
  // this code is lightly adapted from
  //
  // https://chromium.googlesource.com/experimental/chromium/src/+/refs/wip/bajones/webvr_1/chrome/browser/fullscreen_mac.mm
  // and
  // https://chromium.googlesource.com/external/webrtc/+/HEAD/modules/desktop_capture/mac/window_list_utils.cc
  //
  // it tries a few things to detect any fullscreen windows:
  // - first, it checks a couple of presentation options for the
  // currently active app -- see https://developer.apple.com/documentation/appkit/nsapplication/1428717-currentsystempresentationoptions
  // basically, if the active app is hiding the dock and menu, we assume fullscreen mode
  //
  // - second, we check the bounds and positioning of every non-system window. if any of them
  // match or exceed the size of their display, we assume fullscreen mode


  NSApplicationPresentationOptions options =
      [NSApp currentSystemPresentationOptions];
  bool dock_hidden = (options & NSApplicationPresentationHideDock) ||
                      (options & NSApplicationPresentationAutoHideDock);
  bool menu_hidden = (options & NSApplicationPresentationHideMenuBar) ||
                      (options & NSApplicationPresentationAutoHideMenuBar);
  bool result = false;

  if (
    // if the main display has been captured (by games in particular) we are
    // in fullscreen mode
    // NOTE: this call is deprecated so just skipping it
    //    CGDisplayIsCaptured(CGMainDisplayID()) ||

    // If both dock and menu bar are hidden, that is the equivalent of the Carbon
    // SystemUIMode (or Info.plist's LSUIPresentationMode) kUIModeAllHidden.
    (dock_hidden && menu_hidden) ||
    (options & NSApplicationPresentationFullScreen)
    ) {
      result = true;
    }
    else {
      // lets iterate through a bunch of windows
      // and check their bounds against our monitor resolutions

      // get a list of all visible windows
      CFArrayRef windowList = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
      CFIndex numWindows = CFArrayGetCount( windowList );

      // get a list of all the monitors in use
      NSArray<NSScreen *> *screens = [NSScreen screens];

      #ifdef DEBUG_OUTPUT
      char *buffer = (char *)malloc(1200);
      #endif

      // iterate through each window
      for( int i = 0; i < (int)numWindows; i++ ) {
        CFDictionaryRef windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(
          windowList, i);

        // Get the window layer and then skip windows with layer!=0 (menu, dock).
        // As I understand it kCGWindowLayer==0 is basically a 'normal' window
        int layer;
        CFNumberRef window_layer = reinterpret_cast<CFNumberRef>(
            CFDictionaryGetValue(windowInfo, kCGWindowLayer));

        if (!window_layer || 
          !CFNumberGetValue(window_layer, kCFNumberIntType, &layer) || 
          layer != 0) {
          continue;
        }

        // get the bounds of the window
        CFDictionaryRef bounds_ref = reinterpret_cast<CFDictionaryRef>(
          CFDictionaryGetValue(windowInfo, kCGWindowBounds));
        CGRect bounds;

        if (!bounds_ref ||
          !CGRectMakeWithDictionaryRepresentation(bounds_ref, &bounds)) {
          continue;
        }

        #ifdef DEBUG_OUTPUT
        CFStringRef windowName = (CFStringRef)CFDictionaryGetValue(windowInfo, kCGWindowName);
        CFStringGetCString(windowName, buffer, 400, kCFStringEncodingUTF8);
        #endif


        // compare the bounds of this window to the bounds
        // of each screen
        for (NSScreen *screen in screens) {
          NSRect e = [screen frame];
          #ifdef DEBUG_OUTPUT
          std::cout << buffer << " " << bounds.size.width << "x" << bounds.size.height << ":" << bounds.origin.x << "," << bounds.origin.y << " -- " << e.size.width << "x" << e.size.height << ":" << e.origin.x << ","  << e.origin.y << "\n";
          #endif

          //
          // compare the window dimensions to each set of screen dimensions. we assume it might
          // be full screen iff:
          // - the window width and height exceed the screen width/height
          // - the x origins are equal
          // - the y origin of the window is less than or equal to the y origin of the window
          //   which accounts for the possibility of the screen y actually being greater 
          //   than the window y
          if ( bounds.size.width >= e.size.width && 
                bounds.size.height >= e.size.height && 
                bounds.origin.x == e.origin.x && 
                bounds.origin.y <= e.origin.y ) {

            result = true;
            break;
          }
        }
      }

      CFRelease(windowList);
    }

    info.GetReturnValue().Set(result);

  #endif

  #ifdef IS_WINDOWS
	bool result = IsFullScreenWindowMode();
  info.GetReturnValue().Set(result);
  #endif

}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("isFullscreen").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(isFullscreen)).ToLocalChecked());

  // Passing target down to the next NAN_MODULE_INIT
  Fullscreen::Init(target);
}

#if NODE_MAJOR_VERSION >= 10
NAN_MODULE_WORKER_ENABLED(Fullscreen, InitAll);
#else
NODE_MODULE(Fullscreen, InitAll);
#endif