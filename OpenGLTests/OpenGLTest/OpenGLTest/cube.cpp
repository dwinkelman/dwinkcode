#include "functions.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = 180, pitch = 0, fov = 55.0f;

float CAMERA_SPEED = -2.5f, camera_speed;
float last_frame_time = glfwGetTime(), delta_time;

float last_mouse_x = 400, last_mouse_y = 300;

void mouse_callback(GLFWwindow * window, double xpos, double ypos) {
	// Update mouse information
	float sensitivity = 0.05f;
	float x_offset = (xpos - last_mouse_x) * sensitivity;
	float y_offset = (last_mouse_y - ypos) * sensitivity;
	last_mouse_x = xpos;
	last_mouse_y = ypos;
	yaw = fmod(yaw + x_offset, 360.0f);
	pitch += y_offset;
	if (pitch > 89.0f) pitch = 89.0f;
	else if (pitch < -89.0f) pitch = -89.0f;
	camera_front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera_front.y = sin(glm::radians(pitch));
	camera_front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera_front = glm::normalize(camera_front);
}

void scroll_callback(GLFWwindow * window, double x_offset, double y_offset) {
	fov -= y_offset;
	if (fov > 45.0f) fov = 45.0f;
	else if (fov < 1.0f) fov = 1.0f;
}

int test_cube() {
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

	// Initialize the cursor and scroll
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

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
	/*
	   G------H
	  /|     /|
	 / |    / |
	C------D  |
	|  E---|--F     y  z
	| /    | /      ^ /
	|/     |/       |/
	A------B        ---> x
	*/
	float vertices[] = {
		// positions				// colors					// texture coordinates
		-0.5f,	-0.5f,	-0.5f,		1.0f,	0.0f,	0.0f,		0.0f,	0.0f,			// A	R
		0.5f,	-0.5f,	-0.5f,		1.0f,	1.0f,	0.0f,		1.0f,	0.0f,			// B	Y
		-0.5f,	0.5f,	-0.5f,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f,			// C	G
		0.5f,	0.5f,	-0.5f,		0.0f,	0.0f,	1.0f,		1.0f,	1.0f,			// D	B
		-0.5f,	-0.5f,	0.5f,		0.0f,	0.0f,	1.0f,		1.0f,	1.0f,			// E	B
		0.5f,	-0.5f,	0.5f,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f,			// F	G
		-0.5f,	0.5f,	0.5f,		1.0f,	1.0f,	0.0f,		1.0f,	0.0f,			// G	Y
		0.5f,	0.5f,	0.5f,		1.0f,	0.0f,	0.0f,		0.0f,	0.0f,			// H	R
	};
	unsigned int indices[] = {
		// Front
		0, 1, 2,
		1, 2, 3,
		// Right Side
		1, 3, 5,
		3, 5, 7,
		// Left Side
		0, 2, 4,
		2, 4, 6,
		// Bottom
		0, 1, 4,
		1, 4, 5,
		// Top
		2, 3, 6,
		3, 6, 7,
		// Back
		4, 5, 6,
		5, 6, 7
	};
	glm::vec3 offsets[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
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

		// Frame time
		float time = glfwGetTime();
		delta_time = last_frame_time - time;
		last_frame_time = time;
		camera_speed = CAMERA_SPEED * delta_time;

		
		
		// Adjust camera position
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera_pos += camera_speed * camera_front;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera_pos -= camera_speed * camera_front;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;

		// Rendering
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the program
		program.use();

		/*
		// Update the time_color uniform
		float time_val = glfwGetTime();
		float green_val = sin(time_val) / 2.0f + 0.5f;
		int time_color_uniform_loc = glGetUniformLocation(shader_program, "time_color");
		glUniform4f(time_color_uniform_loc, 0.0f, green_val, 0.0f, 1.0f);
		*/

		int model_loc, view_loc, projection_loc;
		model_loc = glGetUniformLocation(program.id, "model");
		view_loc = glGetUniformLocation(program.id, "view");
		projection_loc = glGetUniformLocation(program.id, "projection");

		// Create transformations
		glm::mat4 view, projection;

		float radius = 10.0f;
		float camX = sin(glfwGetTime()) * radius;
		float camZ = cos(glfwGetTime()) * radius;
		view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

		projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

		for (int i = 0; i < 10; i++) {
			glm::mat4 model;
			model = glm::translate(model, offsets[i]);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(70.0f), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

			// Render the shapes
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(vertex_array);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// Swap buffers and check for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Exit the application
	glfwTerminate();
	return 0;
}