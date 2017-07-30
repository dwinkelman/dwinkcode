#version 330
out vec4 out_color;

in vec3 pass_color;
in vec2 pass_tex_coord;

uniform sampler2D tex_img;

void main(){
    out_color = texture(tex_img, pass_tex_coord) * vec4(pass_color, 1.0f);
}