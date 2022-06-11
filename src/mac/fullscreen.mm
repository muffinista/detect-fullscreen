#include "../fullscreen.h"

#include <Carbon/Carbon.h>
#include <AppKit/AppKit.h>
#define NOT_FULLSCREEN_KEY "Menubar"


bool isFullscreenMode() {

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

  // assume full screen mode iff the current app is actively
  // hiding the dock and menu. if they are both set to auto-hide,
  // they might actually be visible so we don't count that
  bool dock_hidden = options & NSApplicationPresentationHideDock;
  bool menu_hidden = options & NSApplicationPresentationHideMenuBar;

  // bool dock_hidden = (options & NSApplicationPresentationHideDock) ||
  //                     (options & NSApplicationPresentationAutoHideDock);
  // bool menu_hidden = (options & NSApplicationPresentationHideMenuBar) ||
  //                     (options & NSApplicationPresentationAutoHideMenuBar);
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
    #ifdef DEBUG_OUTPUT
    std::cout << "active app is hiding menu and dock\n";
    #endif
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

  return result;
}
