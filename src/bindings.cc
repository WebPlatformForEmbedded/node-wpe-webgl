#include <cstdlib>

#include "gles2platform.h"
#include "interface/webgl.h"

extern "C" {
void init(Handle<Object> target)
{
  atexit(gles2platform::AtExit);
  atexit(webgl::WebGLRenderingContext::AtExit);

  Nan::SetMethod(target, "init", gles2platform::init);
  Nan::SetMethod(target, "blit", gles2platform::blit);

  webgl::WebGLRenderingContext::Initialize(target);
}

NODE_MODULE(gles2x11, init)
} // extern "C"
