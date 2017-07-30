#include "shader.h"

Shader::Shader(const GLchar * vertex_path, const GLchar * fragment_path) {
	// Load the sources of the programs
	std::string vertex_src, fragment_src;
	std::ifstream vertex_file, fragment_file;
	vertex_file.exceptions	(std::ifstream::failbit | std::ifstream::badbit);
	fragment_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vertex_file.open(vertex_path);
		fragment_file.open(fragment_path);
		std::stringstream vertex_stream, fragment_stream;
		vertex_stream << vertex_file.rdbuf();
		fragment_stream << fragment_file.rdbuf();
		vertex_file.close();
		fragment_file.close();
		vertex_src = vertex_stream.str();
		fragment_src = fragment_stream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "Error: Shader files not successfully read.\n";
	}
	const char * vertex_code = vertex_src.c_str();
	const char * fragment_code = fragment_src.c_str();

	// Compile the vertex shader
	unsigned int vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_code, NULL);
	glCompileShader(vertex_shader);

	// Compile the fragment shader
	unsigned int fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_code, NULL);
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
	// "id" is the id of the shader program
	id = glCreateProgram();
	glAttachShader(id, vertex_shader);
	glAttachShader(id, fragment_shader);
	glLinkProgram(id);

	// Check that the program linked correctly
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		char info[512];
		glGetProgramInfoLog(id, 512, NULL, info);
		std::cout << "Error: Shader program linking failed:\n" << info << '\n';
	}

	// Clean up compiled shaders
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}