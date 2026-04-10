#include <GLFW/glfw3.h>
#include <stddef.h>
#include <stdlib.h>

int main() {
  int* i = malloc(sizeof(int));
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(800, 600, "test", NULL, NULL);
  glfwDestroyWindow(window);
  glfwTerminate();
}
