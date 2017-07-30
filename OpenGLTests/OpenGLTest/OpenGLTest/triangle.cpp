#include "functions.h"

// GLSL vertex shader source code
const char * BASIC_VERTEX_SHADER_SRC =
"#version 330 core\n"
"layout (location = 0) in vec3 vertex_pos;"
"layout (location = 1) in vec3 in_color;"
""
"out vec3 vertex_color;"
""
"void main(){"
"	gl_Position = vec4(vertex_pos.xyz, 1.0);"
"	vertex_color = in_color;"
"}";

const char * BASIC_FRAGMENT_SHADER_SRC =
"#version 330 core\n"
"out vec3 frag_color;"
""
"in vec3 vertex_color;"
""
"void main(){"
"	frag_color = vertex_color;"
"}";

int test_triangle() {
	// Initialize GLFW parameters
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window
	GLFWwindow * window = glfwCreateWindow(800, 600, "Triangle Test", NULL, NULL);
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
		0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
	};
	unsigned int indices[] = {
		0, 1, 2
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

	// Compile the vertex shader
	unsigned int vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &BASIC_VERTEX_SHADER_SRC, NULL);
	glCompileShader(vertex_shader);

	// Compile the fragment shader
	unsigned int fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &BASIC_FRAGMENT_SHADER_SRC, NULL);
	glCompileShader(fragment_shader);

	// Check that the shaders compiled correctly
	int success;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info[512];
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		std::cout << "Error: Vertex shader compilation failed:\n" << info << '\n';
	}
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info[512];
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		std::cout << "Error: Fragment shader compilation failed:\n" << info << '\n';
	}

	// Link the shaders into a program
	unsigned int shader_program;
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	// Check that the program linked correctly
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		char info[512];
		glGetProgramInfoLog(shader_program, 512, NULL, info);
		std::cout << "Error: Shader program linking failed:\n" << info << '\n';
	}

	// Clean up compiled shaders
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

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
	// Specify color layout
	glVertexAttribPointer(
		1,							// Location of value being passed to ("layout (location = 0) in vec3 aPos;")
		3,							// Number of coordinates in vector
		GL_FLOAT,					// Type of vector
		GL_FALSE,					// Data is NOT normalized
		6 * sizeof(float),			// Stride: space between each vertex
		(void *)(3*sizeof(float))	// Offset from the data
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
		glUseProgram(shader_program);

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
		glBindVertexArray(0);

		// Swap buffers and check for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Exit the application
	glfwTerminate();
	return 0;
}