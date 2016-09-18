#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_st;

uniform mat4 view_mat, proj_mat;

out vec2 texture_coordinates;

void main() {
  texture_coordinates = vertex_st;
  gl_Position = proj_mat * view_mat * vec4(vertex_position, 1.0);
}
