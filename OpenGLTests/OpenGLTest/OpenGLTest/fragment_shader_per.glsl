#version 330 core

in vec3 pass_color;
in vec2 pass_tex_coord;

out vec4 out_color;

uniform sampler2D texture;

void main(){
	out_color = texture(texture, pass_tex_coord);
}