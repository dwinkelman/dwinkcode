#include "functions.h"

int test_shader_class() {
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

	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");

	/*
	// Define vertecies
	float vertices[] = {
	0.5f,	0.5f,	0.0f,	// top right
	0.5f,	-0.5f,	0.0f,	// bottom right
	-0.5f,	-0.5f,	0.0f,	// bottom left
	-0.5f,	0.5f,	0.0f	// top left
	};
	unsigned int indices[] = {
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
	};
	*/
	float vertices[] = {
		// positions         // colors
		0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
		0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,    // top 
		1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f    // side of screen
	};
	unsigned int indices[] = {
		0, 1, 3
	};

	/*
	// Create the vertex buffer
	unsigned int vertex_buf;
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	*/

	// Create the array, buffer, and element buffer
	unsigned int vertex_array;
	glGenVertexArrays(1, &vertex_array);
	unsigned int vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	unsigned int element_buffer;
	glGenBuffers(1, &element_buffer);

	// Bind vertices
	glBindVertexArray(vertex_array);
	// Copy vertices to buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Copy index array to buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// Specify vertex layout
	glVertexAttribPointer(
		0,					// Location of value being passed to ("layout (location = 0) in vec3 aPos;")
		3,					// Number of coordinates in vector
		GL_FLOAT,			// Type of vector
		GL_FALSE,			// Data is NOT normalized
		6 * sizeof(float),	// Stride: space between each vertex
		(void *)0			// Offset from the data
	);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		1,					// Location of value being passed to ("layout (location = 0) in vec3 aPos;")
		3,					// Number of coordinates in vector
		GL_FLOAT,			// Type of vector
		GL_FALSE,			// Data is NOT normalized
		6 * sizeof(float),	// Stride: space between each vertex
		(void *)(3 * sizeof(float))			// Offset from the data
	);
	glEnableVertexAttribArray(1);

	/*
	// Draw in wireframe mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	*/

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		process_input(window);

		// Rendering
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Use the program
		shader.use();

		/*
		// Update the time_color uniform
		float time_val = glfwGetTime();
		float green_val = sin(time_val) / 2.0f + 0.5f;
		int time_color_uniform_loc = glGetUniformLocation(shader_program, "time_color");
		glUniform4f(time_color_uniform_loc, 0.0f, green_val, 0.0f, 1.0f);
		*/

		// Render the shapes
		glBindVertexArray(vertex_array);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// Swap buffers and check for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Exit the application
	glfwTerminate();
	return 0;
}