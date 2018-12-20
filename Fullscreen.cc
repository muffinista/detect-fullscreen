#include <nan.h>
//#include <assert.h>

#ifdef IS_MAC
#include <Carbon/Carbon.h>
#define NOT_FULLSCREEN_KEY "Menubar"
#endif

NAN_METHOD(isFullscreen);

// Example with node ObjectWrap
// Based on https://nodejs.org/api/addons.html#addons_wrapping_c_objects but using NAN
class MyObject : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

  private:
    explicit MyObject();
    ~MyObject();

    static NAN_METHOD(New);
    static Nan::Persistent<v8::Function> constructor;
};


Nan::Persistent<v8::Function> MyObject::constructor;

NAN_MODULE_INIT(MyObject::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("MyObject").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("MyObject").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

//MyObject::MyObject(double value) : value_(value) {
MyObject::MyObject() {
}

MyObject::~MyObject() {
}

NAN_METHOD(MyObject::New) {
  if (info.IsConstructCall()) {
    MyObject *obj = new MyObject();
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


//bool Method(napi_env env, napi_callback_info info) {
NAN_METHOD(isFullscreen) {
  #ifdef IS_MAC
    CGDisplayCount nDisplays;

    CFStringRef str = CFStringCreateWithCString(NULL, NOT_FULLSCREEN_KEY, kCFStringEncodingASCII);
    char *buffer = (char *)malloc(1200);
    unsigned int tally = 0;

    CFArrayRef windowList = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
      
    CFIndex numWindows = CFArrayGetCount( windowList );
      
    for( int i = 0; i < (int)numWindows; i++ ) {
      CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(
        windowList, i);

      CFStringRef windowName = (CFStringRef)CFDictionaryGetValue(
        info, kCGWindowName);

      if (windowName != 0) {
        // if there's a menubar, it's not in fullscreen
        CFStringGetCString(windowName, buffer, 400, kCFStringEncodingUTF8);
        if ( CFStringCompare(windowName, str, 0) == 0 ) {
          tally++;
        }
      }
    }
    CFRelease(windowList);
    CFRelease(str);

    // tally is the count of screens which are not in fullscreen mode

    CGGetActiveDisplayList(0,0, &nDisplays);

    // if nDisplays == tally, then we're not in fullscreen mode
    info.GetReturnValue().Set((nDisplays != tally));
  #endif

  #ifdef IS_WINDOWS
	// v8::Local<v8::Function> cons = Nan::New(constructor);
	// const int argc = 1;
	HWND dskWin = GetDesktopWindow();
	HWND fgWin = GetForegroundWindow();
	bool result = false;

	RECT appBounds;
	RECT rc;

	if ( fgWin != dskWin && fgWin != GetShellWindow() ) {
		GetWindowRect(dskWin, &rc);
		GetWindowRect(fgWin, &appBounds);

		if ( appBounds.left <= rc.left && appBounds.top <= rc.top && appBounds.right >= rc.right && appBounds.bottom >= rc.bottom ) {
			result = true;
		}
	}
	
  info.GetReturnValue().Set(result);
  #endif

}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("isFullscreen").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(isFullscreen)).ToLocalChecked());

  // Passing target down to the next NAN_MODULE_INIT
  MyObject::Init(target);
}

NODE_MODULE(Fullscreen, InitAll);




