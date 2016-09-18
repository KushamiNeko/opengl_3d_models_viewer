#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_st;
layout(location = 3) in vec4 vertex_tangent;

uniform mat4 view_mat, proj_mat;

out vec2 texture_coordinates;
out vec3 normal;

void main() {
  texture_coordinates = vertex_st;
  normal = vertex_normal;
  //gl_Position = proj_mat * view_mat * vec4(vertex_position, 1.0);
  gl_Position = proj_mat * view_mat * vec4(vertex_position, 1.0);
}
