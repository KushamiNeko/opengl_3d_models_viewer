#version 410

// import uniform variable
uniform mat4 view_mat;

// import texture information
// the binding layout is used to explicitly state the sampler variable should
// read from active texture slot 0 (Only work on OpenGL 4.2 and newer)
//layout(binding = 0) uniform sampler2D diff_tex;
// read from active texture slot 0
uniform sampler2D diff_tex;
uniform sampler2D spec_tex;
uniform sampler2D normal_tex;

// attributes pass from vertex shader
in vec2 texture_coordinates;

// eye-space position and normal information from vertex shader
//#in vec3 position_eye, normal_eye;
in vec3 view_dir_tan;
in vec3 light_dir_tan;

// standard output for fragment shader
out vec4 frag_colour;

// point light properties
//#vec3 light_pos_world = vec3(1.0, 2.0, 2.0);
vec3 Ls = vec3(1.0, 1.0, 1.0);
vec3 Ld = vec3(1.0, 1.0, 1.0);
vec3 La = vec3(0.0, 0.0, 0.0);

// surface reflectance properties
//float specular_coefficent = 0.75;
//vec3 Ks = vec3(specular_coefficent, specular_coefficent, specular_coefficent);
//vec3 Kd = vec3(1.0, 1.0, 1.0);
//vec3 Ka = vec3(1.0, 1.0, 1.0);

float Ks = 2;
float Kd = 1.35;
float Ka = 1.0;

float specular_exponent = 15.0;
float normal_map_intensity = 1.5;

void main() {
  // calculate texture sampling
  vec4 diff_texel = texture(diff_tex, texture_coordinates);
  vec4 spec_texel = texture(spec_tex, texture_coordinates);

  // sample normal map and convert from 0:1 range to -1:1 range
  vec3 normal_texel = texture(normal_tex, texture_coordinates).xyz;
  normal_texel = normal_texel * normal_map_intensity + vec3(0.5, 0.5, 1.0)
   * (1.0 - normal_map_intensity);
  normal_texel = normalize(normal_texel * 2.0 - 1);

  // raise the light position to eye space to calculate for further calculation
  //#vec3 light_pos_eye = vec3(view_mat * vec4(light_pos_world, 1.0));
  //#vec3 direction_to_light_eye = normalize(light_pos_eye - position_eye);

  // use reflect function to calculate the reflected light vector for specular
  //#vec3 reflection_eye = reflect(-direction_to_light_eye, normal_eye);
  //#vec3 surface_to_viewer_eye = normalize(-position_eye);
  vec3 reflection_tan = reflect(normalize(light_dir_tan), normal_texel);

  // full phong lighting calculation which is a little bit expensive
  // we can use blin-phong calculation list below to improve the rendering rate

  // full phong lighting model
  // float dot_prod_spec = dot(reflection_eye, surface_to_viewer_eye);
  float dot_prod_spec = dot(reflection_tan, normalize(view_dir_tan));

  // blin-phong lighting model
  //#vec3 half_way_eye = normalize(surface_to_viewer_eye + direction_to_light_eye);
  //#float dot_prod_spec = dot(half_way_eye, normal_eye);

  // dot product clean up for specular
  dot_prod_spec = max(dot_prod_spec, 0.0);
  // use power function to control the specular
  float specular_factor = pow(dot_prod_spec, specular_exponent);

  // use dot product to calculate the diffuse lighting
  // make sure that both direction to light vector and surface normal vector are
  // normalize vefor used in calculation
  //#float dot_prod_diff = dot(direction_to_light_eye, normal_eye);
  vec3 direction_to_light_tan = normalize(-light_dir_tan);
  float dot_prod_diff = dot(direction_to_light_tan, normal_texel);

  // in order to prevent some negative dot_prod value,
  // use a max function to clamp the result here
  dot_prod_diff = max(dot_prod_diff, 0.0);

  // ambient intensity
  vec3 la = La * Ka;

  // diffuse intensity
  vec3 ld = diff_texel.xyz * Ld * Kd * dot_prod_diff;

  // specular intensity
  vec3 ls = spec_texel.xyz * Ls * Ks * specular_factor;

  // final color
  // make sure that the color output does not exceed the 0.0 to 1.0 value range
  vec3 final_color = min(ld + ls + la, 1.0);
  //vec3 final_color = min(normal_texel.xyz, 1.0);

  // frag_colour = diff_texel;
  frag_colour = vec4(final_color, 1.0);
}
