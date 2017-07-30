#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_tex_coord;

out vec3 pass_color;
out vec2 pass_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	gl_Position = projection * view * model * vec4(in_pos, 1.0f);
	pass_color = in_color;
	pass_tex_coord = in_tex_coord;
}