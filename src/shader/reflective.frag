#version 450

in vec2 texture_coordinates;

in vec3 pos_eye;
in vec3 normal_eye;

in vec3 normal;
in vec3 bi_tangent;
in vec4 tangent;

uniform mat4 view_mat, proj_mat;

out vec4 final_color;

///////////////////////////////////////////////////////////////////////////////////////////

uniform sampler2D diff_tex;
uniform sampler2D spec_tex;
uniform sampler2D normal_tex;

uniform samplerCube cube_map_tex;

///////////////////////////////////////////////////////////////////////////////////////////

uniform int use_diffuse_map;
uniform vec3 diff_color;

uniform int use_specular_map;
uniform vec3 spec_color;

uniform int use_normal_map;
uniform float normal_intensity;

///////////////////////////////////////////////////////////////////////////////////////////

const float PI = 3.14159265358979323846;

///////////////////////////////////////////////////////////////////////////////////////////

vec2 Hammersley(uint i, uint N) {
  return vec2(float(i) / float(N),
              float(bitfieldReverse(i)) * 2.3283064365386963e-10);
}

vec3 hemisphereSample_uniform(float u, float v) {
  float phi = v * 2.0 * PI;
  float cosTheta = 1.0 - u;
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
  return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

vec3 hemisphereSample_cos(float u, float v) {
  float phi = v * 2.0 * PI;
  float cosTheta = sqrt(1.0 - u);
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
  return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

vec3 hemisphereSample_cos2(float u1, float u2) {
  float r = sqrt(u1);
  float theta = 2 * PI * u2;

  float x = r * cos(theta);
  float y = r * sin(theta);

  return vec3(x, y, sqrt(max(0.0f, 1 - u1)));
}

vec3 MakeSample(vec2 E) {
  float SineTheta = sin(E.x);

  float x = cos(E.y) * SineTheta;
  float y = sin(E.y) * SineTheta;
  float z = cos(E.x);

  return vec3(x, y, z);
}

///////////////////////////////////////////////////////////////////////////////////////////

float fit01(float src, float newMin, float newMax) {
  float re;

  if (src > 1.0f) {
    re = newMax;

  } else if (src < 0.0f) {
    re = newMin;

  } else {
    float newRange = newMax - newMin;
    re = (src * newRange) + newMin;
  }

  return re;
}

///////////////////////////////////////////////////////////////////////////////////////////

// equation referenced from Disney

// L : surface to light
// V : surface to camera
// N : surface normal

float ior = 2.0;

float Fresnel(float f0, float u) { return f0 + (1 - f0) * pow(1 - u, 5); }

vec3 Fresnel_BRDF(vec3 L, vec3 V, vec3 N) {
  vec3 H = normalize(L + V);
  float f0 = pow((ior - 1) / (ior + 1), 2);
  float f = Fresnel(f0, dot(N, V));
  return vec3(f);
}

float alpha = 0.1;

float sqr(float x) { return x * x; }

float GGX(float alpha, float cosThetaM) {
  float CosSquared = cosThetaM * cosThetaM;
  float TanSquared = (1 - CosSquared) / CosSquared;
  return (1.0 / PI) * sqr(alpha / (CosSquared * (alpha * alpha + TanSquared)));
}

vec3 GGX_BRDF(vec3 L, vec3 V, vec3 N) {
  vec3 H = normalize(L + V);
  float D = GGX(alpha, dot(N, H));
  return vec3(D);
}

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

void main() {
  ///////////////////////////////////////////////////////////////////////////////////////////

  // control for the shader behavoir

  vec3 main_light_world = vec3(1.0, 2.0, 2.0);

  // float reflection_intensity = 0.8;
  float normal_map_intensity = normal_intensity;

  ///////////////////////////////////////////////////////////////////////////////////////////

  // get texture into texel

  vec4 diff_texel;
  vec4 spec_texel;
  vec3 normal_texel;

  if (use_diffuse_map == 1) {
    diff_texel = texture(diff_tex, texture_coordinates);
  } else {
    diff_texel = vec4(diff_color, 1.0);
  }

  if (use_specular_map == 1) {
    spec_texel = texture(spec_tex, texture_coordinates);
  } else {
    spec_texel = vec4(spec_color, 1.0);
  }

  normal_texel = texture(normal_tex, texture_coordinates).xyz;

  // vec4 diff_texel = texture(diff_tex, texture_coordinates);
  // vec4 spec_texel = texture(spec_tex, texture_coordinates);
  // vec3 normal_texel = texture(normal_tex, texture_coordinates).xyz;

  ///////////////////////////////////////////////////////////////////////////////////////////

  // change normal texel into actual normal in eye space

  normal_texel = normal_texel * normal_map_intensity +
                 vec3(0.5, 0.5, 1.0) * (1.0 - normal_map_intensity);

  normal_texel = normalize(normal_texel * 2.0 - 1);

  mat3 t = mat3(tangent.xyz, bi_tangent.xyz, normal.xyz);

  vec3 normal_texel_world = t * normal_texel;
  vec3 normal_texel_eye = vec3(view_mat * vec4(normal_texel_world, 0.0));

  vec3 n_normal_eye;
  if (use_normal_map == 1) {
    n_normal_eye = normalize(normal_texel_eye);
  } else {
    n_normal_eye = normalize(normal_eye);
  }

  // vec3 n_normal_eye = normalize(normal_eye);

  ///////////////////////////////////////////////////////////////////////////////////////////

  // normalize vertex position as vector

  vec3 n_pos_eye = normalize(pos_eye);

  ///////////////////////////////////////////////////////////////////////////////////////////

  // diffuse calculate

  vec3 main_light_eye = vec3(view_mat * vec4(main_light_world, 1.0));
  vec3 n_main_light_direction_eye = normalize(main_light_eye - pos_eye);

  float dot_prod_diff = dot(n_main_light_direction_eye, n_normal_eye);

  ///////////////////////////////////////////////////////////////////////////////////////////

  // specular calculate

  //  vec3 n_reflection_eye =
  //      normalize(reflect(-n_main_light_direction_eye, n_normal_eye));

  vec3 n_surface_to_camera_eye = -n_pos_eye;

  // phong specular model

  //  float dot_prod_spec = dot(n_reflection_eye, n_surface_to_camera_eye);
  //  dot_prod_spec = pow(max(dot_prod_spec, 0.0), 15.0);

  ///////////////////////////////////////////////////////////////////////////////////////////

  // equation referenced from Disney

  vec3 ggx = GGX_BRDF(n_main_light_direction_eye, n_surface_to_camera_eye,
                      n_normal_eye);

  vec3 fresnel = Fresnel_BRDF(n_main_light_direction_eye,
                              n_surface_to_camera_eye, n_normal_eye);

  ///////////////////////////////////////////////////////////////////////////////////////////

  vec3 reflected = reflect(n_pos_eye, n_normal_eye);
  reflected = vec3(inverse(view_mat) * vec4(reflected, 0.0));

  vec3 cube_map_texel = texture(cube_map_tex, reflected).xyz;
  // vec3 cube_map_texel = textureLod(cube_map_tex, reflected, 10).xyz;

  vec3 cube_map_texel_light = cube_map_texel * 1.5;

  vec3 ld =
      diff_texel.xyz * clamp((dot_prod_diff + cube_map_texel_light), 0.0, 1.0);

  // vec3 ls = texture(cube_map_tex, reflected).xyz + ggx;
  vec3 ls = (cube_map_texel + ggx) * spec_texel.xyz * 1.5;

  final_color = vec4(mix(ld, ls, clamp(fresnel, 0.0, 1.0)), 1.0);
  // final_color = vec4(ls, 1.0);

  //  uint numSamples = 32;
  //  vec3 c = vec3(0.0);
  //  for (uint i = 0; i < numSamples; i++) {
  //    vec2 samplePos = Hammersley(i, numSamples);
  //    vec3 hemiVec_tangent = hemisphereSample_cos2(samplePos.x, samplePos.y);
  //
  //    vec3 hemiVec_world = t * hemiVec_tangent;
  //
  //    vec4 ne = view_mat * vec4(normal, 0.0);
  //    vec3 re = reflect(n_pos_eye, ne.xyz);
  //
  //    // vec3 makeSample = MakeSample(samplePos);
  //    c += texture(cube_map_tex, re).xyz;
  //  }

  // final_color = vec4(c / numSamples, 1.0);
}
