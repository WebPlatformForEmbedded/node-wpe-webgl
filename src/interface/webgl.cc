#include <cstring>
#include <vector>
#include <iostream>

#include "webgl.h"
#include <node.h>
#include <node_buffer.h>

#ifdef _WIN32
  #define  strcasestr(s, t) strstr(strupr(s), strupr(t))
#endif

#define CHECK_ARRAY_BUFFER(val) if(!val->IsArrayBufferView()) \
        {Nan::ThrowTypeError("Only support array buffer"); return;}

namespace webgl {

using namespace node;
using namespace v8;
using namespace std;

// forward declarations
enum GLObjectType {
  GLOBJECT_TYPE_BUFFER,
  GLOBJECT_TYPE_FRAMEBUFFER,
  GLOBJECT_TYPE_PROGRAM,
  GLOBJECT_TYPE_RENDERBUFFER,
  GLOBJECT_TYPE_SHADER,
  GLOBJECT_TYPE_TEXTURE,
};

void registerGLObj(GLObjectType type, GLuint obj);
void unregisterGLObj(GLuint obj);

// A 32-bit and 64-bit compatible way of converting a pointer to a GLuint.
static GLuint ToGLuint(const void* ptr) {
  return static_cast<GLuint>(reinterpret_cast<size_t>(ptr));
}

template<typename Type>
inline Type* getArrayData(Local<Value> arg, int* num = NULL) {
  Type *data=NULL;
  if(num) *num=0;

  if(!arg->IsNull()) {
    if(arg->IsArray()) {
      Nan::ThrowError("Not support array type");
    }
    else if(arg->IsObject()) {
      Nan::TypedArrayContents<Type> p(arg);
      data = *p;
      if (num) *num = p.length();
    }
    else
      Nan::ThrowError("Bad array argument");
  }

  return data;
}

inline void *getImageData(Local<Value> arg) {
  void *pixels = NULL;
  if (!arg->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(arg);
    if (!obj->IsObject()){
      Nan::ThrowError("Bad texture argument");
    }else if(obj->IsArrayBufferView()){
        int num;
        pixels = getArrayData<BYTE>(obj, &num);
    }else{
        pixels = node::Buffer::Data(Nan::Get(obj, JS_STR("data")).ToLocalChecked());
    }
  }
  return pixels;
}

Persistent<Function> WebGLRenderingContext::constructor_template;

void WebGLRenderingContext::Initialize (Local<Object> target) {
  Nan::HandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("WebGLRenderingContext"));

  // prototype
  Nan::SetPrototypeMethod(ctor, "uniform1f", Uniform1f);
  Nan::SetPrototypeMethod(ctor, "uniform2f", Uniform2f);
  Nan::SetPrototypeMethod(ctor, "uniform3f", Uniform3f);
  Nan::SetPrototypeMethod(ctor, "uniform4f", Uniform4f);
  Nan::SetPrototypeMethod(ctor, "uniform1i", Uniform1i);
  Nan::SetPrototypeMethod(ctor, "uniform2i", Uniform2i);
  Nan::SetPrototypeMethod(ctor, "uniform3i", Uniform3i);
  Nan::SetPrototypeMethod(ctor, "uniform4i", Uniform4i);
  Nan::SetPrototypeMethod(ctor, "uniform1fv", Uniform1fv);
  Nan::SetPrototypeMethod(ctor, "uniform2fv", Uniform2fv);
  Nan::SetPrototypeMethod(ctor, "uniform3fv", Uniform3fv);
  Nan::SetPrototypeMethod(ctor, "uniform4fv", Uniform4fv);
  Nan::SetPrototypeMethod(ctor, "uniform1iv", Uniform1iv);
  Nan::SetPrototypeMethod(ctor, "uniform2iv", Uniform2iv);
  Nan::SetPrototypeMethod(ctor, "uniform3iv", Uniform3iv);
  Nan::SetPrototypeMethod(ctor, "uniform4iv", Uniform4iv);
  Nan::SetPrototypeMethod(ctor, "pixelStorei", PixelStorei);
  Nan::SetPrototypeMethod(ctor, "bindAttribLocation", BindAttribLocation);
  Nan::SetPrototypeMethod(ctor, "getError", GetError);
  Nan::SetPrototypeMethod(ctor, "drawArrays", DrawArrays);
  Nan::SetPrototypeMethod(ctor, "uniformMatrix2fv", UniformMatrix2fv);
  Nan::SetPrototypeMethod(ctor, "uniformMatrix3fv", UniformMatrix3fv);
  Nan::SetPrototypeMethod(ctor, "uniformMatrix4fv", UniformMatrix4fv);

  Nan::SetPrototypeMethod(ctor, "generateMipmap", GenerateMipmap);

  Nan::SetPrototypeMethod(ctor, "getAttribLocation", GetAttribLocation);
  Nan::SetPrototypeMethod(ctor, "depthFunc", DepthFunc);
  Nan::SetPrototypeMethod(ctor, "viewport", Viewport);
  Nan::SetPrototypeMethod(ctor, "createShader", CreateShader);
  Nan::SetPrototypeMethod(ctor, "shaderSource", ShaderSource);
  Nan::SetPrototypeMethod(ctor, "compileShader", CompileShader);
  Nan::SetPrototypeMethod(ctor, "getShaderParameter", GetShaderParameter);
  Nan::SetPrototypeMethod(ctor, "getShaderInfoLog", GetShaderInfoLog);
  Nan::SetPrototypeMethod(ctor, "createProgram", CreateProgram);
  Nan::SetPrototypeMethod(ctor, "attachShader", AttachShader);
  Nan::SetPrototypeMethod(ctor, "linkProgram", LinkProgram);
  Nan::SetPrototypeMethod(ctor, "getProgramParameter", GetProgramParameter);
  Nan::SetPrototypeMethod(ctor, "getUniformLocation", GetUniformLocation);
  Nan::SetPrototypeMethod(ctor, "clearColor", ClearColor);
  Nan::SetPrototypeMethod(ctor, "clearDepth", ClearDepth);

  Nan::SetPrototypeMethod(ctor, "disable", Disable);
  Nan::SetPrototypeMethod(ctor, "createTexture", CreateTexture);
  Nan::SetPrototypeMethod(ctor, "bindTexture", BindTexture);
  Nan::SetPrototypeMethod(ctor, "texImage2D", TexImage2D);
  Nan::SetPrototypeMethod(ctor, "texParameteri", TexParameteri);
  Nan::SetPrototypeMethod(ctor, "texParameterf", TexParameterf);
  Nan::SetPrototypeMethod(ctor, "clear", Clear);
  Nan::SetPrototypeMethod(ctor, "useProgram", UseProgram);
  Nan::SetPrototypeMethod(ctor, "createFramebuffer", CreateFramebuffer);
  Nan::SetPrototypeMethod(ctor, "bindFramebuffer", BindFramebuffer);
  Nan::SetPrototypeMethod(ctor, "framebufferTexture2D", FramebufferTexture2D);
  Nan::SetPrototypeMethod(ctor, "createBuffer", CreateBuffer);
  Nan::SetPrototypeMethod(ctor, "bindBuffer", BindBuffer);
  Nan::SetPrototypeMethod(ctor, "bufferData", BufferData);
  Nan::SetPrototypeMethod(ctor, "bufferSubData", BufferSubData);
  Nan::SetPrototypeMethod(ctor, "enable", Enable);
  Nan::SetPrototypeMethod(ctor, "blendEquation", BlendEquation);
  Nan::SetPrototypeMethod(ctor, "blendFunc", BlendFunc);
  Nan::SetPrototypeMethod(ctor, "enableVertexAttribArray", EnableVertexAttribArray);
  Nan::SetPrototypeMethod(ctor, "vertexAttribPointer", VertexAttribPointer);
  Nan::SetPrototypeMethod(ctor, "activeTexture", ActiveTexture);
  Nan::SetPrototypeMethod(ctor, "drawElements", DrawElements);
  Nan::SetPrototypeMethod(ctor, "flush", Flush);
  Nan::SetPrototypeMethod(ctor, "finish", Finish);

  Nan::SetPrototypeMethod(ctor, "vertexAttrib1f", VertexAttrib1f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib2f", VertexAttrib2f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib3f", VertexAttrib3f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib4f", VertexAttrib4f);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib1fv", VertexAttrib1fv);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib2fv", VertexAttrib2fv);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib3fv", VertexAttrib3fv);
  Nan::SetPrototypeMethod(ctor, "vertexAttrib4fv", VertexAttrib4fv);

  Nan::SetPrototypeMethod(ctor, "blendColor", BlendColor);
  Nan::SetPrototypeMethod(ctor, "blendEquationSeparate", BlendEquationSeparate);
  Nan::SetPrototypeMethod(ctor, "blendFuncSeparate", BlendFuncSeparate);
  Nan::SetPrototypeMethod(ctor, "clearStencil", ClearStencil);
  Nan::SetPrototypeMethod(ctor, "colorMask", ColorMask);
  Nan::SetPrototypeMethod(ctor, "copyTexImage2D", CopyTexImage2D);
  Nan::SetPrototypeMethod(ctor, "copyTexSubImage2D", CopyTexSubImage2D);
  Nan::SetPrototypeMethod(ctor, "cullFace", CullFace);
  Nan::SetPrototypeMethod(ctor, "depthMask", DepthMask);
  Nan::SetPrototypeMethod(ctor, "depthRange", DepthRange);
  Nan::SetPrototypeMethod(ctor, "disableVertexAttribArray", DisableVertexAttribArray);
  Nan::SetPrototypeMethod(ctor, "hint", Hint);
  Nan::SetPrototypeMethod(ctor, "isEnabled", IsEnabled);
  Nan::SetPrototypeMethod(ctor, "lineWidth", LineWidth);
  Nan::SetPrototypeMethod(ctor, "polygonOffset", PolygonOffset);

  Nan::SetPrototypeMethod(ctor, "sampleCoverage", SampleCoverage);
  Nan::SetPrototypeMethod(ctor, "scissor", Scissor);
  Nan::SetPrototypeMethod(ctor, "stencilFunc", StencilFunc);
  Nan::SetPrototypeMethod(ctor, "stencilFuncSeparate", StencilFuncSeparate);
  Nan::SetPrototypeMethod(ctor, "stencilMask", StencilMask);
  Nan::SetPrototypeMethod(ctor, "stencilMaskSeparate", StencilMaskSeparate);
  Nan::SetPrototypeMethod(ctor, "stencilOp", StencilOp);
  Nan::SetPrototypeMethod(ctor, "stencilOpSeparate", StencilOpSeparate);
  Nan::SetPrototypeMethod(ctor, "bindRenderbuffer", BindRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "createRenderbuffer", CreateRenderbuffer);

  Nan::SetPrototypeMethod(ctor, "deleteBuffer", DeleteBuffer);
  Nan::SetPrototypeMethod(ctor, "deleteFramebuffer", DeleteFramebuffer);
  Nan::SetPrototypeMethod(ctor, "deleteProgram", DeleteProgram);
  Nan::SetPrototypeMethod(ctor, "deleteRenderbuffer", DeleteRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "deleteShader", DeleteShader);
  Nan::SetPrototypeMethod(ctor, "deleteTexture", DeleteTexture);
  Nan::SetPrototypeMethod(ctor, "detachShader", DetachShader);
  Nan::SetPrototypeMethod(ctor, "framebufferRenderbuffer", FramebufferRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "getVertexAttribOffset", GetVertexAttribOffset);

  Nan::SetPrototypeMethod(ctor, "isBuffer", IsBuffer);
  Nan::SetPrototypeMethod(ctor, "isFramebuffer", IsFramebuffer);
  Nan::SetPrototypeMethod(ctor, "isProgram", IsProgram);
  Nan::SetPrototypeMethod(ctor, "isRenderbuffer", IsRenderbuffer);
  Nan::SetPrototypeMethod(ctor, "isShader", IsShader);
  Nan::SetPrototypeMethod(ctor, "isTexture", IsTexture);

  Nan::SetPrototypeMethod(ctor, "renderbufferStorage", RenderbufferStorage);
  Nan::SetPrototypeMethod(ctor, "getShaderSource", GetShaderSource);
  Nan::SetPrototypeMethod(ctor, "validateProgram", ValidateProgram);

  Nan::SetPrototypeMethod(ctor, "texSubImage2D", TexSubImage2D);
  Nan::SetPrototypeMethod(ctor, "readPixels", ReadPixels);
  Nan::SetPrototypeMethod(ctor, "getTexParameter", GetTexParameter);
  Nan::SetPrototypeMethod(ctor, "getActiveAttrib", GetActiveAttrib);
  Nan::SetPrototypeMethod(ctor, "getActiveUniform", GetActiveUniform);
  Nan::SetPrototypeMethod(ctor, "getAttachedShaders", GetAttachedShaders);
  Nan::SetPrototypeMethod(ctor, "getParameter", GetParameter);
  Nan::SetPrototypeMethod(ctor, "getBufferParameter", GetBufferParameter);
  Nan::SetPrototypeMethod(ctor, "getFramebufferAttachmentParameter", GetFramebufferAttachmentParameter);
  Nan::SetPrototypeMethod(ctor, "getProgramInfoLog", GetProgramInfoLog);
  Nan::SetPrototypeMethod(ctor, "getRenderbufferParameter", GetRenderbufferParameter);
  Nan::SetPrototypeMethod(ctor, "getVertexAttrib", GetVertexAttrib);
  Nan::SetPrototypeMethod(ctor, "getSupportedExtensions", GetSupportedExtensions);
  Nan::SetPrototypeMethod(ctor, "getExtension", GetExtension);
  Nan::SetPrototypeMethod(ctor, "checkFramebufferStatus", CheckFramebufferStatus);

  Nan::SetPrototypeMethod(ctor, "frontFace", FrontFace);

  Nan::Set(target, Nan::New("WebGLRenderingContext").ToLocalChecked(), ctor->GetFunction(ctx).ToLocalChecked());

  constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction(ctx).ToLocalChecked());
}

WebGLRenderingContext::WebGLRenderingContext() {
  pixelStorei_UNPACK_FLIP_Y_WEBGL = 0;
  pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0;
  pixelStorei_UNPACK_FLIP_BLUE_RED = 0;
}

NAN_METHOD(WebGLRenderingContext::New) {
  Nan::HandleScope scope;

  WebGLRenderingContext* obj = new WebGLRenderingContext();
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(WebGLRenderingContext::Uniform1f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glUniform1f(location, x);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float y = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glUniform2f(location, x, y);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float y = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float z = (float) info[3]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glUniform3f(location, x, y, z);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4f) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float y = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float z = (float) info[3]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float w = (float) info[4]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glUniform4f(location, x, y, z, w);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform1i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int x = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glUniform1i(location, x);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int x = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int y = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glUniform2i(location, x, y);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int x = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int y = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int z = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glUniform3i(location, x, y, z);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4i) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int x = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int y = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int z = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int w = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glUniform4i(location, x, y, z, w);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform1fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  glUniform1fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 2;

  glUniform2fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 3;

  glUniform3fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4fv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 4;

  glUniform4fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform1iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);

  glUniform1iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform2iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 2;

  glUniform2iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform3iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 3;
  glUniform3iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Uniform4iv) {
  Nan::HandleScope scope;

  int location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 4;
  glUniform4iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::PixelStorei) {
  Nan::HandleScope scope;

  int pname = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int param = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  if (pname == 0x9240 /* UNPACK_FLIP_Y_WEBGL */) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->pixelStorei_UNPACK_FLIP_Y_WEBGL = param;
  } else if (pname == 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL */) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL = param;
  } else if (pname == 0x9245 /* UNPACK_FLIP_BLUE_RED */) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->pixelStorei_UNPACK_FLIP_BLUE_RED = param;
  }

  glPixelStorei(pname,param);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindAttribLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int index = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  Nan::Utf8String name(info[2]);

  glBindAttribLocation(program, index, *name);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetError) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(Nan::New<Integer>(glGetError()));
}


NAN_METHOD(WebGLRenderingContext::DrawArrays) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int first = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int count = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glDrawArrays(mode, first, count);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix2fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLboolean transpose = Nan::To<bool>(info[1]).FromMaybe(true);

  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 4) {
    Nan::ThrowError("Not enough data for UniformMatrix2fv");
  }else{
    glUniformMatrix2fv(location, count / 4, transpose, data);

    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix3fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLboolean transpose = Nan::To<bool>(info[1]).FromMaybe(true);
  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 9) {
    Nan::ThrowError("Not enough data for UniformMatrix3fv");
  }else{
    glUniformMatrix3fv(location, count / 9, transpose, data);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix4fv) {
  Nan::HandleScope scope;

  GLint location = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLboolean transpose = Nan::To<bool>(info[1]).FromMaybe(true);
  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 16) {
    Nan::ThrowError("Not enough data for UniformMatrix4fv");
  }else{
    glUniformMatrix4fv(location, count / 16, transpose, data);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::GenerateMipmap) {
  Nan::HandleScope scope;

  GLint target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  glGenerateMipmap(target);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetAttribLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  Nan::Utf8String name(info[1]);

  info.GetReturnValue().Set(Nan::New<Number>(glGetAttribLocation(program, *name)));
}


NAN_METHOD(WebGLRenderingContext::DepthFunc) {
  Nan::HandleScope scope;

  glDepthFunc(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Viewport) {
  Nan::HandleScope scope;

  int x = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int y = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int width = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int height = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glViewport(x, y, width, height);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateShader) {
  Nan::HandleScope scope;

  GLuint shader=glCreateShader(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  #ifdef LOGGING
  cout<<"createShader "<<shader<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_SHADER, shader);
  info.GetReturnValue().Set(Nan::New<Number>(shader));
}


NAN_METHOD(WebGLRenderingContext::ShaderSource) {
  Nan::HandleScope scope;

  int id = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  Nan::Utf8String code(info[1]);

  const char* codes[1];
  codes[0] = *code;
  GLint length=code.length();

  glShaderSource  (id, 1, codes, &length);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CompileShader) {
  Nan::HandleScope scope;

  glCompileShader(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::FrontFace) {
  Nan::HandleScope scope;

  glFrontFace(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetShaderParameter) {
  Nan::HandleScope scope;

  int shader = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int value = 0;
  switch (pname) {
  case GL_DELETE_STATUS:
  case GL_COMPILE_STATUS:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value!=0)));
    break;
  case GL_SHADER_TYPE:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<unsigned long>(value)));
    break;
  case GL_INFO_LOG_LENGTH:
  case GL_SHADER_SOURCE_LENGTH:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
    break;
  default:
    Nan::ThrowTypeError("GetShaderParameter: Invalid Enum");
  }
  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetShaderInfoLog) {
  Nan::HandleScope scope;

  int id = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int Len = 1024;
  char Error[1024];
  glGetShaderInfoLog(id, 1024, &Len, Error);

  info.GetReturnValue().Set(JS_STR(Error));
}


NAN_METHOD(WebGLRenderingContext::CreateProgram) {
  Nan::HandleScope scope;

  GLuint program=glCreateProgram();
  #ifdef LOGGING
  cout<<"createProgram "<<program<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_PROGRAM, program);
  info.GetReturnValue().Set(Nan::New<Number>(program));
}


NAN_METHOD(WebGLRenderingContext::AttachShader) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int shader = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glAttachShader(program, shader);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::LinkProgram) {
  Nan::HandleScope scope;

  glLinkProgram(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetProgramParameter) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  int value = 0;
  switch (pname) {
  case GL_DELETE_STATUS:
  case GL_LINK_STATUS:
  case GL_VALIDATE_STATUS:
    glGetProgramiv(program, pname, &value);
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value!=0)));
    break;
  case GL_ATTACHED_SHADERS:
  case GL_ACTIVE_ATTRIBUTES:
  case GL_ACTIVE_UNIFORMS:
    glGetProgramiv(program, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
    break;
  default:
    Nan::ThrowTypeError("GetProgramParameter: Invalid Enum");
  }
  //info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetUniformLocation) {
  Nan::HandleScope scope;

  int program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  Nan::Utf8String name(info[1]);

  info.GetReturnValue().Set(JS_INT(glGetUniformLocation(program, *name)));
}


NAN_METHOD(WebGLRenderingContext::ClearColor) {
  Nan::HandleScope scope;

  float red = (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float green = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float blue = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float alpha = (float) info[3]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glClearColor(red, green, blue, alpha);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::ClearDepth) {
  Nan::HandleScope scope;

  float depth = (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glClearDepthf(depth);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Disable) {
  Nan::HandleScope scope;

  glDisable(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Enable) {
  Nan::HandleScope scope;

  glEnable(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CreateTexture) {
  Nan::HandleScope scope;

  GLuint texture;
  glGenTextures(1, &texture);
  #ifdef LOGGING
  cout<<"createTexture "<<texture<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_TEXTURE, texture);
  info.GetReturnValue().Set(Nan::New<Number>(texture));
}


NAN_METHOD(WebGLRenderingContext::BindTexture) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int texture = info[1]->IsNull() ? 0 : info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBindTexture(target, texture);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexImage2D) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int level = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int internalformat = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int width = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int height = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int border = info[5]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int format = info[6]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int type = info[7]->Int32Value(Nan::GetCurrentContext()).FromJust();
  void *pixels=getImageData(info[8]);

  if (pixels) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->preprocessTexImageData(pixels, width, height, format, type);
  }

  glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::TexParameteri) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int param = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glTexParameteri(target, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexParameterf) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float param = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glTexParameterf(target, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Clear) {
  Nan::HandleScope scope;

  glClear(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::UseProgram) {
  Nan::HandleScope scope;

  glUseProgram(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateBuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  glGenBuffers(1, &buffer);
  #ifdef LOGGING
  cout<<"createBuffer "<<buffer<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_BUFFER, buffer);
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}

NAN_METHOD(WebGLRenderingContext::BindBuffer) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int buffer = info[1]->Uint32Value(Nan::GetCurrentContext()).FromJust();
  glBindBuffer(target,buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CreateFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  glGenFramebuffers(1, &buffer);
  #ifdef LOGGING
  cout<<"createFrameBuffer "<<buffer<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_FRAMEBUFFER, buffer);
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}


NAN_METHOD(WebGLRenderingContext::BindFramebuffer) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int buffer = info[1]->IsNull() ? 0 : info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBindFramebuffer(target, buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::FramebufferTexture2D) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int attachment = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int textarget = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int texture = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int level = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glFramebufferTexture2D(target, attachment, textarget, texture, level);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BufferData) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  if(info[1]->IsObject()) {
    Local<Object> obj = Local<Object>::Cast(info[1]);
    GLenum usage = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

    CHECK_ARRAY_BUFFER(obj);

    int element_size = 1;
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = arr->ByteLength()* element_size;
    void* data = arr->Buffer()->GetContents().Data();

    glBufferData(target, size, data, usage);
  }
  else if(info[1]->IsNumber()) {
    GLsizeiptr size = info[1]->Uint32Value(Nan::GetCurrentContext()).FromJust();
    GLenum usage = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
    glBufferData(target, size, NULL, usage);
  }
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BufferSubData) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int offset = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  Local<Object> obj = Local<Object>::Cast(info[2]);

   int element_size = 1;
   Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
   int size = arr->ByteLength()* element_size;
   void* data = arr->Buffer()->GetContents().Data();

  glBufferSubData(target, offset, size, data);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BlendEquation) {
  Nan::HandleScope scope;

  int mode=info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBlendEquation(mode);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BlendFunc) {
  Nan::HandleScope scope;

  int sfactor=info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int dfactor=info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBlendFunc(sfactor,dfactor);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::EnableVertexAttribArray) {
  Nan::HandleScope scope;

  glEnableVertexAttribArray(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::VertexAttribPointer) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int size = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int type = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int normalized = Nan::To<bool>(info[3]).FromJust();
  int stride = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();
  long offset = info[5]->Int32Value(Nan::GetCurrentContext()).FromJust();

  //    printf("VertexAttribPointer %d %d %d %d %d %d\n", indx, size, type, normalized, stride, offset);
  glVertexAttribPointer(indx, size, type, normalized, stride, (const GLvoid *)offset);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::ActiveTexture) {
  Nan::HandleScope scope;

  glActiveTexture(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::DrawElements) {
  Nan::HandleScope scope;

  int mode = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int count = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int type = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLvoid *offset = reinterpret_cast<GLvoid*>(info[3]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  glDrawElements(mode, count, type, offset);
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Flush) {
  Nan::HandleScope scope;
  glFlush();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Finish) {
  Nan::HandleScope scope;
  glFinish();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib1f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glVertexAttrib1f(indx, x);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib2f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float y = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glVertexAttrib2f(indx, x, y);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib3f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float y = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float z = (float) info[3]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glVertexAttrib3f(indx, x, y, z);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib4f) {
  Nan::HandleScope scope;

  GLuint indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  float x = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float y = (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float z = (float) info[3]->NumberValue(Nan::GetCurrentContext()).FromJust();
  float w = (float) info[4]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glVertexAttrib4f(indx, x, y, z, w);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib1fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib1fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib2fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib2fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib3fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib3fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib4fv) {
  Nan::HandleScope scope;

  int indx = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib4fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendColor) {
  Nan::HandleScope scope;

  GLclampf r= (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();
  GLclampf g= (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();
  GLclampf b= (float) info[2]->NumberValue(Nan::GetCurrentContext()).FromJust();
  GLclampf a= (float) info[3]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glBlendColor(r,g,b,a);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendEquationSeparate) {
  Nan::HandleScope scope;

  GLenum modeRGB= info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum modeAlpha= info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBlendEquationSeparate(modeRGB,modeAlpha);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendFuncSeparate) {
  Nan::HandleScope scope;

  GLenum srcRGB= info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum dstRGB= info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum srcAlpha= info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum dstAlpha= info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBlendFuncSeparate(srcRGB,dstRGB,srcAlpha,dstAlpha);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ClearStencil) {
  Nan::HandleScope scope;

  GLint s = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glClearStencil(s);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ColorMask) {
  Nan::HandleScope scope;

  GLboolean r = Nan::To<bool>(info[0]).FromJust();
  GLboolean g = Nan::To<bool>(info[1]).FromJust();
  GLboolean b = Nan::To<bool>(info[2]).FromJust();
  GLboolean a = Nan::To<bool>(info[3]).FromJust();

  glColorMask(r,g,b,a);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CopyTexImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint level = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum internalformat = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint x = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint y = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei width = info[5]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei height = info[6]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint border = info[7]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glCopyTexImage2D( target, level, internalformat, x, y, width, height, border);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CopyTexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint level = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint xoffset = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint yoffset = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint x = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint y = info[5]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei width = info[6]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei height = info[7]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CullFace) {
  Nan::HandleScope scope;

  GLenum mode = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glCullFace(mode);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DepthMask) {
  Nan::HandleScope scope;

  GLboolean flag = Nan::To<bool>(info[0]).FromJust();

  glDepthMask(flag);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DepthRange) {
  Nan::HandleScope scope;

  GLclampf zNear = (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();
  GLclampf zFar = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glDepthRangef(zNear, zFar);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DisableVertexAttribArray) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glDisableVertexAttribArray(index);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Hint) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum mode = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glHint(target, mode);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::IsEnabled) {
  Nan::HandleScope scope;

  GLenum cap = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  bool ret=glIsEnabled(cap)!=0;
  info.GetReturnValue().Set(Nan::New<Boolean>(ret));
}

NAN_METHOD(WebGLRenderingContext::LineWidth) {
  Nan::HandleScope scope;

  GLfloat width = (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glLineWidth(width);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::PolygonOffset) {
  Nan::HandleScope scope;

  GLfloat factor = (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();
  GLfloat units = (float) info[1]->NumberValue(Nan::GetCurrentContext()).FromJust();

  glPolygonOffset(factor, units);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::SampleCoverage) {
  Nan::HandleScope scope;

  GLclampf value = (float) info[0]->NumberValue(Nan::GetCurrentContext()).FromJust();
  GLboolean invert = Nan::To<bool>(info[1]).FromJust();

  glSampleCoverage(value, invert);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Scissor) {
  Nan::HandleScope scope;

  GLint x = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint y = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei width = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei height = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glScissor(x, y, width, height);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilFunc) {
  Nan::HandleScope scope;

  GLenum func = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint ref = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint mask = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glStencilFunc(func, ref, mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilFuncSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum func = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint ref = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint mask = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glStencilFuncSeparate(face, func, ref, mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilMask) {
  Nan::HandleScope scope;

  GLuint mask = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glStencilMask(mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilMaskSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint mask = info[1]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glStencilMaskSeparate(face, mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilOp) {
  Nan::HandleScope scope;

  GLenum fail = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum zfail = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum zpass = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glStencilOp(fail, zfail, zpass);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilOpSeparate) {
  Nan::HandleScope scope;

  GLenum face = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum fail = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum zfail = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum zpass = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glStencilOpSeparate(face, fail, zfail, zpass);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint buffer = info[1]->IsNull() ? 0 : info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  glBindRenderbuffer(target, buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffers;
  glGenRenderbuffers(1,&renderbuffers);
  #ifdef LOGGING
  cout<<"createRenderBuffer "<<renderbuffers<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_RENDERBUFFER, renderbuffers);
  info.GetReturnValue().Set(Nan::New<Number>(renderbuffers));
}

NAN_METHOD(WebGLRenderingContext::DeleteBuffer) {
  Nan::HandleScope scope;

  GLuint buffer = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDeleteBuffers(1,&buffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDeleteFramebuffers(1,&buffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteProgram) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDeleteProgram(program);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffer = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDeleteRenderbuffers(1, &renderbuffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteShader) {
  Nan::HandleScope scope;

  GLuint shader = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDeleteShader(shader);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteTexture) {
  Nan::HandleScope scope;

  GLuint texture = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDeleteTextures(1,&texture);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DetachShader) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();
  GLuint shader = info[1]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glDetachShader(program, shader);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::FramebufferRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum attachment = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum renderbuffertarget = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint renderbuffer = info[3]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetVertexAttribOffset) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();
  GLenum pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  void *ret=NULL;

  glGetVertexAttribPointerv(index, pname, &ret);
  info.GetReturnValue().Set(JS_INT(ToGLuint(ret)));
}

NAN_METHOD(WebGLRenderingContext::IsBuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(Nan::New<Boolean>(glIsBuffer(info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsFramebuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsFramebuffer(info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsProgram) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsProgram(info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsRenderbuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsRenderbuffer( info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsShader) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsShader(info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust())!=0));
}

NAN_METHOD(WebGLRenderingContext::IsTexture) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsTexture(info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust())!=0));
}

NAN_METHOD(WebGLRenderingContext::RenderbufferStorage) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum internalformat = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei width = info[2]->Uint32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei height = info[3]->Uint32Value(Nan::GetCurrentContext()).FromJust();

  glRenderbufferStorage(target, internalformat, width, height);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetShaderSource) {
  Nan::HandleScope scope;

  int shader = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  GLint len;
  glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &len);
  GLchar *source=new GLchar[len];
  glGetShaderSource(shader, len, NULL, source);

  Local<String> str = JS_STR(source);
  delete[] source;

  info.GetReturnValue().Set(str);
}

NAN_METHOD(WebGLRenderingContext::ValidateProgram) {
  Nan::HandleScope scope;

  glValidateProgram(info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint level = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint xoffset = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint yoffset = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei width = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei height = info[5]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum format = info[6]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum type = info[7]->Int32Value(Nan::GetCurrentContext()).FromJust();
  void *pixels=getImageData(info[8]);

  if (pixels) {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    obj->preprocessTexImageData(pixels, width, height, format, type);
  }

  glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ReadPixels) {
  Nan::HandleScope scope;

  GLint x = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLint y = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei width = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLsizei height = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum format = info[4]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum type = info[5]->Int32Value(Nan::GetCurrentContext()).FromJust();
  void *pixels=getImageData(info[6]);

  glReadPixels(x, y, width, height, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetTexParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  GLint param_value=0;
  glGetTexParameteriv(target, pname, &param_value);

  info.GetReturnValue().Set(Nan::New<Number>(param_value));
}

NAN_METHOD(WebGLRenderingContext::GetActiveAttrib) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint index = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  char name[1024];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  glGetActiveAttrib(program, index, 1024, &length, &size, &type, name);

  Local<Array> activeInfo = Nan::New<Array>(3);
  Nan::Set(activeInfo, JS_STR("size"), JS_INT(size));
  Nan::Set(activeInfo, JS_STR("type"), JS_INT((int)type));
  Nan::Set(activeInfo, JS_STR("name"), JS_STR(name));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(WebGLRenderingContext::GetActiveUniform) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint index = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  char name[1024];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  glGetActiveUniform(program, index, 1024, &length, &size, &type, name);

  Local<Array> activeInfo = Nan::New<Array>(3);
  Nan::Set(activeInfo, JS_STR("size"), JS_INT(size));
  Nan::Set(activeInfo, JS_STR("type"), JS_INT((int)type));
  Nan::Set(activeInfo, JS_STR("name"), JS_STR(name));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(WebGLRenderingContext::GetAttachedShaders) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  GLuint shaders[1024];
  GLsizei count;
  glGetAttachedShaders(program, 1024, &count, shaders);

  Local<Array> shadersArr = Nan::New<Array>(count);
  for(int i=0;i<count;i++)
    Nan::Set(shadersArr, i, JS_INT((int)shaders[i]));

  info.GetReturnValue().Set(shadersArr);
}

NAN_METHOD(WebGLRenderingContext::GetParameter) {
  Nan::HandleScope scope;

  GLenum name = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  switch(name) {
  case GL_BLEND:
  case GL_CULL_FACE:
  case GL_DEPTH_TEST:
  case GL_DEPTH_WRITEMASK:
  case GL_DITHER:
  case GL_POLYGON_OFFSET_FILL:
  case GL_SAMPLE_COVERAGE_INVERT:
  case GL_SCISSOR_TEST:
  case GL_STENCIL_TEST:
  case 0x9240 /* UNPACK_FLIP_Y_WEBGL */:
  {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    info.GetReturnValue().Set(JS_BOOL(obj->pixelStorei_UNPACK_FLIP_Y_WEBGL!=0));
    break;
  }
  case 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL*/:
  {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    info.GetReturnValue().Set(JS_BOOL(obj->pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL!=0));
    break;
  }
  case 0x9245 /* BGRA*/:
  {
    WebGLRenderingContext* obj = ObjectWrap::Unwrap<WebGLRenderingContext>(info.Holder());
    info.GetReturnValue().Set(JS_BOOL(obj->pixelStorei_UNPACK_FLIP_BLUE_RED!=0));
    break;
  }
  case GL_DEPTH_CLEAR_VALUE:
  case GL_LINE_WIDTH:
  case GL_POLYGON_OFFSET_FACTOR:
  case GL_POLYGON_OFFSET_UNITS:
  case GL_SAMPLE_COVERAGE_VALUE:
  {
    // return a float
    GLfloat params;
    ::glGetFloatv(name, &params);
    info.GetReturnValue().Set(JS_FLOAT(params));
    break;
  }
  case GL_RENDERER:
  case GL_SHADING_LANGUAGE_VERSION:
  case GL_VENDOR:
  case GL_VERSION:
  case GL_EXTENSIONS:
  {
    // return a string
    char *params=(char*) ::glGetString(name);

    if(params!=NULL){
      info.GetReturnValue().Set(JS_STR(params));
    }else{
      info.GetReturnValue().Set(Nan::Undefined());
    }

    break;
  }
  case GL_MAX_VIEWPORT_DIMS:
  {
    // return a int32[2]
    GLint params[2];
    ::glGetIntegerv(name, params);

    Local<Array> arr=Nan::New<Array>(2);
    Nan::Set(arr, 0, JS_INT(params[0]));
    Nan::Set(arr, 1, JS_INT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_SCISSOR_BOX:
  case GL_VIEWPORT:
  {
    // return a int32[4]
    GLint params[4];
    ::glGetIntegerv(name, params);

    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0, JS_INT(params[0]));
    Nan::Set(arr, 1, JS_INT(params[1]));
    Nan::Set(arr, 2, JS_INT(params[2]));
    Nan::Set(arr, 3, JS_INT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ALIASED_LINE_WIDTH_RANGE:
  case GL_ALIASED_POINT_SIZE_RANGE:
  case GL_DEPTH_RANGE:
  {
    // return a float[2]
    GLfloat params[2];
    ::glGetFloatv(name, params);
    Local<Array> arr=Nan::New<Array>(2);
    Nan::Set(arr, 0, JS_FLOAT(params[0]));
    Nan::Set(arr, 1, JS_FLOAT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_BLEND_COLOR:
  case GL_COLOR_CLEAR_VALUE:
  {
    // return a float[4]
    GLfloat params[4];
    ::glGetFloatv(name, params);
    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0, JS_FLOAT(params[0]));
    Nan::Set(arr, 1, JS_FLOAT(params[1]));
    Nan::Set(arr, 2, JS_FLOAT(params[2]));
    Nan::Set(arr, 3, JS_FLOAT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_COLOR_WRITEMASK:
  {
    // return a boolean[4]
    GLboolean params[4];
    ::glGetBooleanv(name, params);
    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0, JS_BOOL(params[0]==1));
    Nan::Set(arr, 1, JS_BOOL(params[1]==1));
    Nan::Set(arr, 2, JS_BOOL(params[2]==1));
    Nan::Set(arr, 3, JS_BOOL(params[3]==1));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ARRAY_BUFFER_BINDING:
  case GL_CURRENT_PROGRAM:
  case GL_ELEMENT_ARRAY_BUFFER_BINDING:
  case GL_FRAMEBUFFER_BINDING:
  case GL_RENDERBUFFER_BINDING:
  case GL_TEXTURE_BINDING_2D:
  case GL_TEXTURE_BINDING_CUBE_MAP:
  {
    GLint params;
    ::glGetIntegerv(name, &params);
    info.GetReturnValue().Set(JS_INT(params));
    break;
  }
  default: {
    // return a long
    GLint params;
    ::glGetIntegerv(name, &params);
    info.GetReturnValue().Set(JS_INT(params));
  }
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetBufferParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  GLint params;
  glGetBufferParameteriv(target,pname,&params);
  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(WebGLRenderingContext::GetFramebufferAttachmentParameter) {
  Nan::HandleScope scope;

  GLenum target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum attachment = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLenum pname = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();

  GLint params;
  glGetFramebufferAttachmentParameteriv(target,attachment, pname,&params);
  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(WebGLRenderingContext::GetProgramInfoLog) {
  Nan::HandleScope scope;

  GLuint program = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int Len = 1024;
  char Error[1024];
  glGetProgramInfoLog(program, 1024, &Len, Error);

  info.GetReturnValue().Set(JS_STR(Error));
}

NAN_METHOD(WebGLRenderingContext::GetRenderbufferParameter) {
  Nan::HandleScope scope;

  int target = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  int value = 0;
  glGetRenderbufferParameteriv(target,pname,&value);

  info.GetReturnValue().Set(JS_INT(value));
}

NAN_METHOD(WebGLRenderingContext::GetVertexAttrib) {
  Nan::HandleScope scope;

  GLuint index = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  GLuint pname = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();

  GLint value=0;

  switch (pname) {
  case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
  case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_BOOL(value!=0));
    break;
  case GL_VERTEX_ATTRIB_ARRAY_SIZE:
  case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
  case GL_VERTEX_ATTRIB_ARRAY_TYPE:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_INT(value));
    break;
  case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_INT(value));
    break;
  case GL_CURRENT_VERTEX_ATTRIB: {
    float vextex_attribs[4];
    glGetVertexAttribfv(index,pname,vextex_attribs);
    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0, JS_FLOAT(vextex_attribs[0]));
    Nan::Set(arr, 1, JS_FLOAT(vextex_attribs[1]));
    Nan::Set(arr, 2, JS_FLOAT(vextex_attribs[2]));
    Nan::Set(arr, 3, JS_FLOAT(vextex_attribs[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  default:
    Nan::ThrowError("GetVertexAttrib: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetSupportedExtensions) {
  Nan::HandleScope scope;

  char *extensions=(char*) glGetString(GL_EXTENSIONS);

  info.GetReturnValue().Set(JS_STR(extensions));
}

// TODO GetExtension(name) return the extension name if found, should be an object...
NAN_METHOD(WebGLRenderingContext::GetExtension) {
  Nan::HandleScope scope;

  Nan::Utf8String name(info[0]);
  char *sname=*name;
  char *extensions=(char*) glGetString(GL_EXTENSIONS);
  char *ext=strcasestr(extensions, sname);

  if(ext==NULL){
      info.GetReturnValue().Set(Nan::Undefined());
  }else{
     info.GetReturnValue().Set(JS_STR(ext, (int)::strlen(sname)));
  }
}

NAN_METHOD(WebGLRenderingContext::CheckFramebufferStatus) {
  Nan::HandleScope scope;

  GLenum target=info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();

  info.GetReturnValue().Set(JS_INT((int)glCheckFramebufferStatus(target)));
}

void WebGLRenderingContext::preprocessTexImageData(void * pixels, int width, int height, int format, int type) {
  if (pixelStorei_UNPACK_FLIP_BLUE_RED) {
    if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
      Nan::ThrowError("UNPACK_FLIP_BLUE_RED is only implemented for format RGBA and type UNSIGNED_BYTE");
    }
    int total = width * height * 4;
    unsigned char * data = (unsigned char *) pixels;
    for (int o = 0; o < total; o += 4) {
      unsigned char red = data[o];
      data[o] = data[o+2];
      data[o+2] = red;
    }
  }

  if (pixelStorei_UNPACK_PREMULTIPLY_ALPHA_WEBGL) {
    if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
      Nan::ThrowError("UNPACK_PREMULTIPLY_ALPHA_WEBGL is only implemented for format RGBA and type UNSIGNED_BYTE");
    }
    int total = width * height * 4;
    unsigned char * data = (unsigned char *) pixels;
    for (int o = 0; o < total; o += 4) {
      unsigned char alpha = data[o + 3];
      data[o] = (data[o] * alpha) >> 8;
      data[o+1] = (data[o+1] * alpha) >> 8;
      data[o+2] = (data[o+2] * alpha) >> 8;
    }
  }

  if (pixelStorei_UNPACK_FLIP_Y_WEBGL) {
    Nan::ThrowError("UNPACK_FLIP_Y_WEBGL is not implemented");
  }
}

struct GLObj {
  GLObjectType type;
  GLuint obj;
  GLObj(GLObjectType type, GLuint obj) {
    this->type=type;
    this->obj=obj;
  }
};

vector<GLObj*> globjs;
static bool atExit=false;

void registerGLObj(GLObjectType type, GLuint obj) {
  globjs.push_back(new GLObj(type,obj));
}


void unregisterGLObj(GLuint obj) {
  if(atExit) return;

  vector<GLObj*>::iterator it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    if(globj->obj==obj) {
      delete globj;
      globjs.erase(it);
      break;
    }
    ++it;
  }
}

void WebGLRenderingContext::AtExit() {
  atExit=true;
  //glFinish();

  vector<GLObj*>::iterator it;

  #ifdef LOGGING
  cout<<"WebGL AtExit() called"<<endl;
  cout<<"  # objects allocated: "<<globjs.size()<<endl;
  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *obj=*it;
    cout<<"[";
    switch(obj->type) {
    case GLOBJECT_TYPE_BUFFER: cout<<"buffer"; break;
    case GLOBJECT_TYPE_FRAMEBUFFER: cout<<"framebuffer"; break;
    case GLOBJECT_TYPE_PROGRAM: cout<<"program"; break;
    case GLOBJECT_TYPE_RENDERBUFFER: cout<<"renderbuffer"; break;
    case GLOBJECT_TYPE_SHADER: cout<<"shader"; break;
    case GLOBJECT_TYPE_TEXTURE: cout<<"texture"; break;
    };
    cout<<": "<<obj->obj<<"] ";
    ++it;
  }
  cout<<endl;
  #endif

  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    GLuint obj=globj->obj;

    switch(globj->type) {
    case GLOBJECT_TYPE_PROGRAM:
      #ifdef LOGGING
      cout<<"  Destroying GL program "<<obj<<endl;
      #endif
      glDeleteProgram(obj);
      break;
    case GLOBJECT_TYPE_BUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL buffer "<<obj<<endl;
      #endif
      glDeleteBuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_FRAMEBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL frame buffer "<<obj<<endl;
      #endif
      glDeleteFramebuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_RENDERBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL render buffer "<<obj<<endl;
      #endif
      glDeleteRenderbuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_SHADER:
      #ifdef LOGGING
      cout<<"  Destroying GL shader "<<obj<<endl;
      #endif
      glDeleteShader(obj);
      break;
    case GLOBJECT_TYPE_TEXTURE:
      #ifdef LOGGING
      cout<<"  Destroying GL texture "<<obj<<endl;
      #endif
      glDeleteTextures(1,&obj);
      break;
    default:
      #ifdef LOGGING
      cout<<"  Unknown object "<<obj<<endl;
      #endif
      break;
    }
    delete globj;
    ++it;
  }

  globjs.clear();
}

} // end namespace webgl
