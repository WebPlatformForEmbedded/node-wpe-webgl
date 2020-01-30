#include <cstring>
#include <vector>
#include <iostream>

#include "gles2platform.h"
#include "gles2impl.h"

namespace gles2platform {

using namespace node;
using namespace v8;
using namespace std;

NAN_METHOD(init) {
  Nan::HandleScope scope;

  int width = Nan::To<uint32_t>(info[0]).FromMaybe(0);
  int height = Nan::To<uint32_t>(info[1]).FromMaybe(0);
  bool fullscreen = Nan::To<bool>(info[2]).FromMaybe(true);

  Nan::Utf8String title(info[3]);
  unsigned int layer = Nan::To<uint32_t>(info[4]).FromMaybe(0);

  std::string message = gles2impl::init(width, height, fullscreen, *title, layer);
  if (message.size()) {
    Nan::ThrowRangeError(message.c_str());
  }

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(nextFrame) {
  Nan::HandleScope scope;

  bool swapBuffers = Nan::To<bool>(info[0]).FromMaybe(true);

  gles2impl::nextFrame(swapBuffers);

  info.GetReturnValue().Set(Nan::Undefined());
}

void AtExit() {
  gles2impl::cleanup();
}

} // end namespace gles2platform
