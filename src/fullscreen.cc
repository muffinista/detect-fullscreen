#include <node.h>
#include "fullscreen.h"

namespace Fullscreen {

  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::Boolean;
  using v8::Value;

  void isFullscreen(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    bool isFullscreen = isFullscreenMode();
    args.GetReturnValue().Set(Boolean::New(isolate, static_cast<double>(isFullscreen)));
  }

  void init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "isFullscreen", isFullscreen);
  }

  NODE_MODULE(isFullscreen, init)
}  // Fullscreen
