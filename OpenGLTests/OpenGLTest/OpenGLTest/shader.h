#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Shader {
public:
	// ID of the program
	unsigned int id = 0;
	Shader(const GLchar * vertex_path, const GLchar * fragment_path);
	void use() {
		glUseProgram(id);
	}
	void setBool(const std::string & var_name, bool value) {
		glUniform1i(glGetUniformLocation(id, var_name.c_str()), (GLint)value);
	}
	void setInt(const std::string & var_name, int value) {
		glUniform1i(glGetUniformLocation(id, var_name.c_str()), value);
	}
	void setFloat(const std::string & var_name, float value) {
		glUniform1f(glGetUniformLocation(id, var_name.c_str()), value);
	}
	void setFloatVec4(const std::string & var_name, float v0, float v1, float v2, float v3) {
		glUniform4f(glGetUniformLocation(id, var_name.c_str()), v0, v1, v2, v3);
	}
};

#endif