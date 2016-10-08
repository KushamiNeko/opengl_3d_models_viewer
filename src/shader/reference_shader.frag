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

// baseColor .82 .67 .16;
float metallic = 0;
float subsurface = 0;
float specular = .5;
float roughness = .5;
float specularTint = 0;
float anisotropic = 0;
float sheen = 0;
float sheenTint = .5;
float clearcoat = 1;
float clearcoatGloss = 1;

float ior = 1.6;

const float PI = 3.14159265358979323846;

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

// L : surface to light
// V : surface to camera
// N : surface normal

// X : surface tangent
// Y : surface bitangent

float sqr(float x) { return x * x; }

float SchlickFresnel(float u) {
  float f0 = pow((ior - 1) / (ior + 1), 2);
  return f0 + (1 - f0) * pow(1 - u, 5);
}

// float SchlickFresnel(float u) {
//  float m = clamp(1 - u, 0, 1);
//  float m2 = m * m;
//  return m2 * m2 * m;  // pow(m,5)
//}

float GTR1(float NdotH, float a) {
  if (a >= 1) return 1 / PI;
  float a2 = a * a;
  float t = 1 + (a2 - 1) * NdotH * NdotH;
  return (a2 - 1) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a) {
  float a2 = a * a;
  float t = 1 + (a2 - 1) * NdotH * NdotH;
  return a2 / (PI * t * t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
  return 1 / (PI * ax * ay *
              sqr(sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH));
}

float smithG_GGX(float Ndotv, float alphaG) {
  float a = alphaG * alphaG;
  float b = Ndotv * Ndotv;
  return 1 / (Ndotv + sqrt(a + b - a * b));
}

vec3 mon2lin(vec3 x) {
  return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, vec3 X, vec3 Y) {
  // vec3 BRDF(vec3 L, vec3 V, vec3 N) {
  vec4 diff_texel = texture(diff_tex, texture_coordinates);
  vec4 spec_texel = texture(spec_tex, texture_coordinates);
  vec3 normal_texel = texture(normal_tex, texture_coordinates).xyz;

  float NdotL = dot(N, L);
  float NdotV = dot(N, V);
  // if (NdotL < 0 || NdotV < 0) return vec3(0);

  vec3 H = normalize(L + V);
  float NdotH = dot(N, H);
  float LdotH = dot(L, H);

  // vec3 Cdlin = mon2lin(baseColor);
  vec3 Cdlin = mon2lin(diff_texel.xyz);
  float Cdlum =
      .3 * Cdlin[0] + .6 * Cdlin[1] + .1 * Cdlin[2];  // luminance approx.

  vec3 Ctint =
      Cdlum > 0 ? Cdlin / Cdlum : vec3(1);  // normalize lum. to isolate hue+sat
  vec3 Cspec0 =
      mix(specular * .08 * mix(vec3(1), Ctint, specularTint), Cdlin, metallic);
  vec3 Csheen = mix(vec3(1), Ctint, sheenTint);

  // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
  // and mix in diffuse retro-reflection based on roughness
  float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
  float Fd90 = 0.5 + 2 * LdotH * LdotH * roughness;
  float Fd = mix(1, Fd90, FL) * mix(1, Fd90, FV);

  // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
  // 1.25 scale is used to (roughly) preserve albedo
  // Fss90 used to "flatten" retroreflection based on roughness
  float Fss90 = LdotH * LdotH * roughness;
  float Fss = mix(1, Fss90, FL) * mix(1, Fss90, FV);
  float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

  // specular
  float aspect = sqrt(1 - anisotropic * .9);
  float ax = max(.001, sqr(roughness) / aspect);
  float ay = max(.001, sqr(roughness) * aspect);
  float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
  float FH = SchlickFresnel(LdotH);
  vec3 Fs = mix(Cspec0, vec3(1), FH);
  float roughg = sqr(roughness * .5 + .5);
  float Gs = smithG_GGX(NdotL, roughg) * smithG_GGX(NdotV, roughg);

  // sheen
  vec3 Fsheen = FH * sheen * Csheen;

  // clearcoat (ior = 1.5 -> F0 = 0.04)
  float Dr = GTR1(NdotH, mix(.1, .001, clearcoatGloss));
  float Fr = mix(.04, 1.0, FH);
  float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

  return vec3(Gr * Dr);
  return vec3(0.25 * clearcoat * Gr * Fr * Dr);

  return ((1 / PI) * mix(Fd, ss, subsurface) * Cdlin + Fsheen) *
             (1 - metallic) +
         Gs * Fs * Ds + .25 * clearcoat * Gr * Fr * Dr;
}

void main() {
  vec3 main_light_world = vec3(1.0, 2.0, 2.0);

  vec3 main_light_eye = vec3(view_mat * vec4(main_light_world, 1.0));
  vec3 n_pos_eye = normalize(pos_eye);

  vec3 n_main_light_direction_eye = normalize(main_light_eye - pos_eye);
  vec3 n_surface_to_camera_eye = -n_pos_eye;
  vec3 n_normal_eye = normalize(normal_eye);

  vec3 n_tangent_eye = normalize(view_mat * vec4(tangent.xyz, 0.0)).xyz;
  vec3 n_bi_tangent_eye = normalize(view_mat * vec4(bi_tangent, 0.0)).xyz;

  final_color = vec4(BRDF(n_main_light_direction_eye, n_surface_to_camera_eye,
                          n_normal_eye, n_tangent_eye, n_bi_tangent_eye),
                     1.0);
}
