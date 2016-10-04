#version 410

in vec2 texture_coordinates;

in vec3 pos_eye;
in vec3 normal_eye;

in vec3 normal;
in vec3 bi_tangent;
in vec4 tangent;

uniform mat4 view_mat, proj_mat;

out vec4 final_color;

uniform sampler2D diff_tex;
uniform sampler2D spec_tex;
uniform sampler2D normal_tex;

uniform samplerCube cube_map_tex;

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

float ior = 1.6;

float Fresnel(float f0, float u) { return f0 + (1 - f0) * pow(1 - u, 5); }

vec3 Fresnel_BRDF(vec3 L, vec3 V, vec3 N) {
  vec3 H = normalize(L + V);
  float f0 = pow((ior - 1) / (ior + 1), 2);
  float f = Fresnel(f0, dot(N, V));
  // float f = SchlickFresnel(dot(N, V));
  return vec3(f);
}

float alpha = 0.1;
const float PI = 3.14159265358979323846;

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

// L : surface to light
// V : surface to camera
// N : surface normal

///////////////////////////////////////////////////////////////////////////////////////////

// vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
//  vec4 color = vec4(0.0);
//  vec2 off1 = vec2(1.411764705882353) * direction;
//  vec2 off2 = vec2(3.2941176470588234) * direction;
//  vec2 off3 = vec2(5.176470588235294) * direction;
//  color += texture2D(image, uv) * 0.1964825501511404;
//  color += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;
//  color += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;
//  color += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;
//  color += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;
//  color += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;
//  color += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;
//  return color;
//}
//
//// const vec2 gaussFilter[7] = {-3.0,     0.015625, -2.0,   0.09375, -1.0,
////                             0.234375, 0.0,      0.3125, 1.0,     0.234375,
////                             2.0,      0.09375,  3.0,    0.015625};
////
//// vec3 gaussBlur(sampler2D tex, vec2 coordinates) {
////  vec4 color = 0.0;
////  for (int i = 0; i < 7; i++) {
////    //    color += texture2D(u_Texture0,
////    //                       vec2(v_Coordinates.x + gaussFilter[i].x *
////    //                       u_Scale.x,
////    //                            v_Coordinates.y + gaussFilter[i].x *
////    //                            u_Scale.y))
////    //                            *
////    //             gaussFilter[i].y;
////
////    color += texture(tex, vec2(coordinates.x + gaussFilter[i].x * 1,
////                               coordinates.y + gaussFilter[i].x * 1)) *
////             gaussFilter[i].y;
////  }
////
////  return color.xyz;
////}
//
// vec3 gaussBlur(sampler2D tex, vec2 texture_coordinates, vec2 dir, float
// radius,
//               float resolution) {
//  // this will be our RGBA sum
//  vec4 sum = vec4(0.0);
//
//  // our original texcoord for this fragment
//  vec2 tc = texture_coordinates;
//
//  // the amount to blur, i.e. how far off center to sample from
//  // 1.0 -> blur by one pixel
//  // 2.0 -> blur by two pixels, etc.
//  float blur = radius / resolution;
//
//  // the direction of our blur
//  //(1.0, 0.0) -> x-axis blur
//  //(0.0, 1.0) -> y-axis blur
//  float hstep = dir.x;
//  float vstep = dir.y;
//
//  // apply blurring, using a 9-tap filter with predefined gaussian weights
//
//  sum += texture2D(tex,
//                   vec2(tc.x - 4.0 * blur * hstep, tc.y - 4.0 * blur * vstep))
//                   *
//         0.0162162162;
//  sum += texture2D(tex,
//                   vec2(tc.x - 3.0 * blur * hstep, tc.y - 3.0 * blur * vstep))
//                   *
//         0.0540540541;
//  sum += texture2D(tex,
//                   vec2(tc.x - 2.0 * blur * hstep, tc.y - 2.0 * blur * vstep))
//                   *
//         0.1216216216;
//  sum += texture2D(tex,
//                   vec2(tc.x - 1.0 * blur * hstep, tc.y - 1.0 * blur * vstep))
//                   *
//         0.1945945946;
//
//  sum += texture2D(tex, vec2(tc.x, tc.y)) * 0.2270270270;
//
//  sum += texture2D(tex,
//                   vec2(tc.x + 1.0 * blur * hstep, tc.y + 1.0 * blur * vstep))
//                   *
//         0.1945945946;
//  sum += texture2D(tex,
//                   vec2(tc.x + 2.0 * blur * hstep, tc.y + 2.0 * blur * vstep))
//                   *
//         0.1216216216;
//  sum += texture2D(tex,
//                   vec2(tc.x + 3.0 * blur * hstep, tc.y + 3.0 * blur * vstep))
//                   *
//         0.0540540541;
//  sum += texture2D(tex,
//                   vec2(tc.x + 4.0 * blur * hstep, tc.y + 4.0 * blur * vstep))
//                   *
//         0.0162162162;
//
//  // discard alpha for our simple demo, multiply by vertex color and return
//  // gl_FragColor = vColor * vec4(sum.rgb, 1.0);
//  return sum.xyz;
//}

///////////////////////////////////////////////////////////////////////////////////////////

void main() {
  ///////////////////////////////////////////////////////////////////////////////////////////

  // control for the shader behavoir

  vec3 main_light_world = vec3(1.0, 2.0, 2.0);

  float reflection_intensity = 0.8;
  float normal_map_intensity = 1.0;

  ///////////////////////////////////////////////////////////////////////////////////////////

  // get texture into texel

  vec4 diff_texel = texture(diff_tex, texture_coordinates);
  vec4 spec_texel = texture(spec_tex, texture_coordinates);
  vec3 normal_texel = texture(normal_tex, texture_coordinates).xyz;

  ///////////////////////////////////////////////////////////////////////////////////////////

  // change normal texel into actual normal in eye space

  normal_texel = normal_texel * normal_map_intensity +
                 vec3(0.5, 0.5, 1.0) * (1.0 - normal_map_intensity);

  normal_texel = normalize(normal_texel * 2.0 - 1);

  mat3 t = mat3(tangent.xyz, bi_tangent.xyz, normal.xyz);

  vec3 normal_world = t * normal_texel;
  vec3 normal_eye = vec3(view_mat * vec4(normal_world, 0.0));
  vec3 n_normal_eye = normalize(normal_eye);

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

  vec3 n_reflection_eye =
      normalize(reflect(-n_main_light_direction_eye, n_normal_eye));

  vec3 n_surface_to_camera_eye = -n_pos_eye;

  float dot_prod_spec = dot(n_reflection_eye, n_surface_to_camera_eye);
  dot_prod_spec = pow(max(dot_prod_spec, 0.0), 15.0);

  ///////////////////////////////////////////////////////////////////////////////////////////

  // equation referenced from Disney

  vec3 ggx =
      GGX_BRDF(n_main_light_direction_eye, n_surface_to_camera_eye, normal_eye);

  vec3 fresnel = Fresnel_BRDF(n_main_light_direction_eye,
                              n_surface_to_camera_eye, normal_eye);

  ///////////////////////////////////////////////////////////////////////////////////////////

  vec3 reflected = reflect(n_pos_eye, n_normal_eye);
  reflected = vec3(inverse(view_mat) * vec4(reflected, 0.0));

  vec3 cube_map_textel = texture(cube_map_tex, reflected).xyz;

  vec3 cube_map_textel_light = cube_map_textel * 1.5;

  vec3 ld =
      diff_texel.xyz * clamp((dot_prod_diff + cube_map_textel_light), 0.0, 1.0);

  // vec3 ls = texture(cube_map_tex, reflected).xyz + ggx;
  vec3 ls = cube_map_textel + ggx;

  final_color = vec4(mix(ld, ls, clamp(fresnel, 0.0, 1.0)), 1.0);
  // final_color = vec4(ls, 1.0);
}
