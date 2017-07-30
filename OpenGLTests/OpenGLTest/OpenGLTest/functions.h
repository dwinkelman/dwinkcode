#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>

#include "shader.h"

#define BASE_DIR "C:/Cygwin/home/Daniel/c++/OpenGLTests/OpenGLTest/OpenGLTest/"

/* Callbacks */
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void process_input(GLFWwindow * window);

/* Basic test: creating a window. */
int test_hello();

/* Drawing a 2D triangle */
int test_triangle();

/* Using the shader class */
int test_shader_class();

/* Texture testing using STB */
int test_texture();

/* Perspective testing using a texture */
int test_perspective();

/* Perspective testing using a cube */
int test_cube();

#endif