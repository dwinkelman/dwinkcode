#include "functions.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

int test_perspective() {
	// Initialize GLFW parameters
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window
	GLFWwindow * window = glfwCreateWindow(800, 600, "Texture Test", NULL, NULL);
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

	// Enable depth buffering
	glEnable(GL_DEPTH_TEST);

	// Load and compile shaders
	Shader program(
		BASE_DIR"shaders/vertex_shader_per.glsl",
		BASE_DIR"shaders/fragment_shader_per.glsl");

	// Generate a texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Put options here wrapping/filtering texture

	// Load the image
	int width, height, n_channels;
	unsigned char * data = stbi_load(BASE_DIR"images/wood.png", &width, &height, &n_channels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Error: could not load texture.\n";
	}
	// Free image from memory
	stbi_image_free(data);

	// Create buffers
	unsigned int vertex_array;
	glGenVertexArrays(1, &vertex_array);
	unsigned int vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	unsigned int element_buffer;
	glGenBuffers(1, &element_buffer);

	// Define verticies with color and texture data
	float vertices[] = {
		// positions				// colors				// texture coords
		0.5f,	0.5f,	0.0f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,   // top right
		0.5f,	-0.5f,	0.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,   // bottom right
		-0.5f,	-0.5f,	0.0f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,   // bottom left
		-0.5f,	0.5f,	0.0f,		1.0f, 1.0f, 0.0f,		0.0f, 1.0f    // top left
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	// Bind vertices
	glBindVertexArray(vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Define data layout
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		std::this_thread::sleep_for(std::chrono::microseconds(10));

		// Input
		process_input(window);

		// Rendering
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Use the program
		program.use();

		/*
		// Update the time_color uniform
		float time_val = glfwGetTime();
		float green_val = sin(time_val) / 2.0f + 0.5f;
		int time_color_uniform_loc = glGetUniformLocation(shader_program, "time_color");
		glUniform4f(time_color_uniform_loc, 0.0f, green_val, 0.0f, 1.0f);
		*/

		// Create transformations
		glm::mat4 model, view, projection;
		model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		
		int model_loc, view_loc, projection_loc;
		model_loc = glGetUniformLocation(program.id, "model");
		view_loc = glGetUniformLocation(program.id, "view");
		projection_loc = glGetUniformLocation(program.id, "projection");
		//std::cout << "Model: " << model_loc << ", View: " << view_loc << ", Projection: " << projection_loc << '\n';
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

		// Render the shapes
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(vertex_array);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Swap buffers and check for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Exit the application
	glfwTerminate();
	return 0;
}