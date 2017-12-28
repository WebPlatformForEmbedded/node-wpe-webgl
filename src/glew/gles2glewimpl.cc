#include <cstring>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdio.h>

#include "../gles2impl.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

namespace gles2impl {

GLFWwindow* window;

string init(int width, int height, bool fullscreen, std::string title, unsigned int layer) {
  printf("initializing GLEW\n");

  if (!glfwInit()) {
    return string("Can't init GLEW\n");
  }

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(width, height, title.c_str(), fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
  if (!window) {
      return string("Can't create window.");
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  glewInit();

  return string("");
}

void nextFrame(bool swapBuffers) {
  if (glfwWindowShouldClose(window)) {
    exit(0);
  }

  if (swapBuffers) {
    glfwSwapBuffers(window);
  }

  glfwPollEvents();
}

void cleanup() {
  glfwTerminate();

  printf("cleanup\n");
}

} // end namespace gles2impl