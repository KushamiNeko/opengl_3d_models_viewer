#version 410

// note:

// get attributes from c codes on location 0, which should be consistent with
// the index we specified when we call glVertexAttribPointer() on that buffer

// the "in" keyword means that the variable will get value from our c codes

// the "out" keyword means that the variable will pass down the pipeline to be
// used in fragment shader and so on

// the "layout" keyword is only compatible with OPENGL 4

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

// import texture coordinates attributes
layout(location = 2) in vec2 vertex_st;

// import tangent attributes
layout(location = 3) in vec4 vertex_tangent;

// uniform attributes pass from c codes
// if we gonna move the model, than we have to use a model matrix
// uniform variable as well
uniform mat4 view_mat, proj_mat;

// output the eye-space vertex position and normal to the fragment shader
//#out vec3 position_eye, normal_eye;
out vec3 view_dir_tan;
out vec3 light_dir_tan;
out vec2 texture_coordinates;

void main() {
  // pass down the texture coordinates attributes
  texture_coordinates = vertex_st;

  // raise the vertex position and normal vector into eye-space for simplifying
  // lighting calculation
  // In eye-space, camera position is the origin

  //#position_eye = vec3(view_mat * vec4(vertex_position, 1.0));

  // make sure the normal information is normalized after we change the vector
  // space or doing other calculation
  //#normal_eye = normalize(vec3(view_mat * vec4(vertex_normal, 0.0)));

  gl_Position = proj_mat * view_mat * vec4(vertex_position, 1.0);
  //#gl_Position = proj_mat * vec4(position_eye, 1.0);

  // get camera position by calculating the view matrix
  vec3 cam_pos_loc = (inverse(view_mat) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

  // direction equals -1 * light position
  vec3 light_dir_loc = vec3(-1.0, -2.0, -2.0);

  // get bi-tangent by calculating cross product of normal and tangent, then
  // multiply the result by the determinant, which we store in w
  vec3 bi_tangent = cross(vertex_normal, vertex_tangent.xyz) * vertex_tangent.w;

  vec3 view_dir_loc = normalize(cam_pos_loc - vertex_position);
  view_dir_tan = vec3(dot(vertex_tangent.xyz, view_dir_loc),
   dot(bi_tangent, view_dir_loc), dot(vertex_normal, view_dir_loc));

  light_dir_tan = vec3(dot(vertex_tangent.xyz, light_dir_loc),
  dot(bi_tangent, light_dir_loc), dot(vertex_normal, light_dir_loc));
}
