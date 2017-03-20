#include <cstring>
#include <vector>
#include <iostream>
#include <stdio.h>

#include "../gles2impl.h"

//#include "bcm_host.h"

#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

using namespace std;

namespace gles2impl {

string init(int width, int height, bool fullscreen, std::string title) {
  printf("initializing Nexus\n");


  return string("");
}

void nextFrame(bool swapBuffers) {
}

void cleanup() {

  printf("cleanup\n");
}

} // end namespace gles2impl
