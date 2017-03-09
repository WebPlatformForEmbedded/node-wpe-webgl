#include <cstring>
#include <vector>
#include <iostream>

#include "gles2platform.h"

// We need to split this due to conflicts in X11 header files with v8 header files.
#include "gles2impl.h"

namespace gles2platform {

using namespace node;
using namespace v8;
using namespace std;

NAN_METHOD(init) {
  Nan::HandleScope scope;

  int width = info[0]->Int32Value();
  int height = info[1]->Int32Value();

  std::string message = gles2impl::init(width, height);
  if (message.size()) {
    Nan::ThrowRangeError(message.c_str());
  }

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(blit) {
  Nan::HandleScope scope;

  gles2impl::blit();

  info.GetReturnValue().Set(Nan::Undefined());
}

void AtExit() {
  gles2impl::cleanup();
}

} // end namespace gles2platform
