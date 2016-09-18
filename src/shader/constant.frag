#version 410

in vec2 texture_coordinates;
in vec3 normal;

out vec4 final_color;

uniform sampler2D diff_tex;
uniform sampler2D spec_tex;
uniform sampler2D normal_tex;

void main() {
  vec4 diff_texel = texture(diff_tex, texture_coordinates);
  vec4 spec_texel = texture(spec_tex, texture_coordinates);
  vec4 normal_texel = texture(normal_tex, texture_coordinates);

  final_color = vec4(normal_texel.xyz, 1.0f);
  //final_color = vec4(normal, 1.0f);
}
