#include <cstring>
#include <vector>
#include <iostream>

#include "../gles2impl.h"

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

using namespace std;

namespace gles2impl {

Display    *x_display;
Window      win;
EGLDisplay  egl_display;
EGLContext  egl_context;
EGLSurface  egl_surface;

string init(int width, int height) {
  printf("initializing X11 & EGL\n");
  
  x_display = XOpenDisplay ( NULL );   // open the standard display (the primary screen)
  if ( x_display == NULL ) {
  	return string("cannot connect to X server");
  }
  
  Window root  =  DefaultRootWindow( x_display );   // get the root window (usually the whole screen)
  
  XSetWindowAttributes  swa;
  swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
  
  win  =  XCreateWindow (   // create a window with the provided parameters
      x_display, root,
      0, 0, width, height,   0,
      CopyFromParent, InputOutput,
      CopyFromParent, CWEventMask,
      &swa );
  
  XSetWindowAttributes  xattr;

  xattr.override_redirect = False;
  XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

  XWMHints hints;
  hints.input = True;
  hints.flags = InputHint;
  XSetWMHints(x_display, win, &hints);

  // make the window visible on the screen
  XMapWindow (x_display, win);
  XStoreName (x_display, win, "test");

  // get identifiers for the provided atom name strings
  Atom wm_state = XInternAtom (x_display, "_NET_WM_STATE", False);

  XEvent xev;
  memset ( &xev, 0, sizeof(xev) );
  xev.type         = ClientMessage;
  xev.xclient.window     = win;
  xev.xclient.message_type = wm_state;
  xev.xclient.format     = 32;
  xev.xclient.data.l[0]  = 1;
  xev.xclient.data.l[1]  = False;
  XSendEvent (
    x_display,
    DefaultRootWindow ( x_display ),
    False,
    SubstructureNotifyMask,
    &xev
  );
     
  ///////  the egl part  //////////////////////////////////////////////////////////////////
  //  egl provides an interface to connect the graphics related functionality of openGL ES
  //  with the windowing interface and functionality of the native operation system (X11
  //  in our case.
  
  egl_display  =  eglGetDisplay( (EGLNativeDisplayType) x_display );
  if ( egl_display == EGL_NO_DISPLAY ) {
  	return string("Got no EGL display");
  }
  
  if ( !eglInitialize( egl_display, NULL, NULL ) ) {
  	return string("Unable to initialize EGL");
  }
  
  EGLint attr[] = {       // some attributes to set up our egl-interface
      EGL_BUFFER_SIZE, 16,
      EGL_RENDERABLE_TYPE,
      EGL_OPENGL_ES2_BIT,
      EGL_NONE
  };
  
  EGLConfig  ecfg;
  EGLint     num_config;
  if ( !eglChooseConfig( egl_display, attr, &ecfg, 1, &num_config ) ) {
  	return string("Failed to choose config (eglError: ") + to_string(eglGetError()) + string(")");
  }
  
  if ( num_config != 1 ) {
  	return string("Didn't get exactly one config, but ") + to_string(num_config);
  }
  
  egl_surface = eglCreateWindowSurface ( egl_display, ecfg, win, NULL );
  if ( egl_surface == EGL_NO_SURFACE ) {
  	return string("Unable to create EGL surface (eglError: ") + to_string(eglGetError()) + string(")");
  }
  
  //// egl-contexts collect all state descriptions needed required for operation
  EGLint ctxattr[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
  };
  egl_context = eglCreateContext ( egl_display, ecfg, EGL_NO_CONTEXT, ctxattr );
  if ( egl_context == EGL_NO_CONTEXT ) {
  	return string("Unable to create EGL context (eglError: ") + to_string(eglGetError()) + string(")");
  }
  
  //// associate the egl-context with the egl-surface
  eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context );

  return string("");
}

void blit() {
  eglSwapBuffers ( egl_display, egl_surface );  // get the rendered buffer to the screen
}

void cleanup() {
  eglDestroyContext ( egl_display, egl_context );
  eglDestroySurface ( egl_display, egl_surface );
  eglTerminate      ( egl_display );
  XDestroyWindow    ( x_display, win );
  XCloseDisplay     ( x_display );

  printf("cleanup\n");
}

} // end namespace gles2impl
