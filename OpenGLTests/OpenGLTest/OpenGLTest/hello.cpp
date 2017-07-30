#include "functions.h"

int test_hello() {
	// Initialize GLFW parameters
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window
	GLFWwindow * window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
	if (window == NULL) {
		std::cout << "Error: Could not create window.\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Error: Failed to initialize GLAD.\n";
		return -1;
	}

	// Set viewport
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		process_input(window);

		// Rendering
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Swap buffers and check for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Exit the application
	glfwTerminate();
	return 0;
}