#version 410

in vec2 texture_coordinates;
out vec4 final_color;

uniform sampler2D diff_tex;

void main() {
  vec4 diff_texel = texture(diff_tex, texture_coordinates);
  final_color = vec4(diff_texel.xyz, 1.0f);
}
