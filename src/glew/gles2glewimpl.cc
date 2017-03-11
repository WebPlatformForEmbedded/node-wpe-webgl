#include <cstring>
#include <vector>
#include <iostream>

#include "../gles2impl.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

namespace gles2impl {

GLFWwindow* window;

string init(int width, int height) {
  printf("initializing GLEW\n");

  if (!glfwInit()) {
    return string("Can't init GLEW\n");
  }

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(width, height, "", NULL, NULL);
  if (!window) {
      return string("Can't create window.");
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  glewInit();

  return string("");
}

void blit() {
  //glfwWindowShouldClose(window)?

  glfwSwapBuffers(window);
}

void cleanup() {
  glfwTerminate();

  printf("cleanup\n");
}

} // end namespace gles2impl
