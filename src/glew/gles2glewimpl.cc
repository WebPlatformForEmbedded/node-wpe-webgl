#include <cstring>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "../gles2impl.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <uv.h>

using namespace std;

namespace gles2impl {

GLFWwindow* window;


void poll(void *arg) {
  while(true) {
    printf("poll\n");
    glfwPollEvents();
    usleep(10000); // Every 10msec.
  }
}

string init(int width, int height, bool fullscreen, std::string title) {
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

  // Start the events polling.
  uv_thread_t pollThreadId;
  int a = 0;
  uv_thread_create(&pollThreadId, poll, &a);


  return string("");
}


void blit() {
  //glfwWindowShouldClose(window)?

  glfwSwapBuffers(window);
  glfwPollEvents();
}

void cleanup() {
  glfwTerminate();

  printf("cleanup\n");
}

} // end namespace gles2impl