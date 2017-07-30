#include "functions.h"

void framebuffer_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, 800, 600);
}

void process_input(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}