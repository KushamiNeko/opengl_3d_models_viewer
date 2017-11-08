// include GLEW header before GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <math.h>

/* #include <stb_image.h> */

/* #define STB_IMAGE_WRITE_IMPLEMENTATION */
/* #include "../../third_party/stb/stb_image_write.h" */

/* #include "helper.h" */
#include "linear_math.h"
#include "obj_parser.h"

#include "../../general/src/general_helper.h"

/* #ifndef __DEBUG */
/* #include "debug.h" */
/* #endif */

#define VERTEX_SHADER_FILE "src/shader/vertex_shader.vert"
#define FRAGMENT_SHADER_FILE "src/shader/fragment_shader.frag"

#define OBJ_FILE "./geo/05.obj"
#define OBJ_DIFFUSE_MAP "./geo/05_color.jpg"
#define OBJ_SPECULAR_MAP "./geo/05_reflect.jpg"
#define OBJ_NORMAL_MAP "./geo/05_normalbump.jpg"

// #define OBJ_FILE "./geo/man/test.obj"
// #define OBJ_DIFFUSE_MAP "./geo/man/Diffuse_Colour.jpg"
// #define OBJ_SPECULAR_MAP "./geo/man/Spec_Amount.jpg"
// #define OBJ_NORMAL_MAP "./geo/man/Normal.jpg"

#define ANTI_ALIAS_LEVEL 4

/* #if __DEBUG */
/* void program_link_check(GLuint program) { */
/*   printf("glLinkProgram check\n"); */
/*   glLinkProgram(program); */

/*   // error of program link check */
/*   int params = -1; */
/*   glGetProgramiv(program, GL_LINK_STATUS, &params); */
/*   if (GL_TRUE != params) { */
/*     fprintf(stderr, "ERROR: could not link shader program with index %u\n",
 */
/*             program); */
/*     _print_program_info_log(program); */
/*     exit(1); */
/*   } */
/* } */

/* void shader_compile_check(GLuint shader) { */
/*   printf("glCompileShader check\n"); */
/*   glCompileShader(shader); */

/*   // check for shader compile error */
/*   int params = -1; */
/*   glGetShaderiv(shader, GL_COMPILE_STATUS, &params); */
/*   if (GL_TRUE != params) { */
/*     fprintf(stderr, "ERROR: GL Sahder %i did not compile\n", shader); */
/*     _print_shader_info_log(shader); */
/*     exit(1); */
/*   } */
/* } */

/* #define __LinkProgram(expr) program_link_check(expr) */
/* #define __CompileShader(expr) shader_compile_check(expr) */
/* #else */
/* #define __LinkProgram(expr) glLinkProgram(expr) */
/* #define __CompileShader(expr) glCompileShader(expr) */
/* #endif */

#define GL_MAJOR_VERSION 4
#define GL_MINOR_VERSION 3

void glfw_error_callback(int error, const char *description);
void glfw_window_size_callback(GLFWwindow *window, int width, int height);

// void moving_triangle(GLuint program, float *matrix, int matrix_loc);
bool camera_control(GLFWwindow *window, float *cam_pos, float *cam_yaw,
                    float cam_speed, float cam_yaw_speed,
                    double elapsed_seconds);

// window resize tracking variable
static int gl_window_width = 1280;
static int gl_window_height = 720;

static void glfw_error_callback(int error, const char *description) {
  printf("GLFW ERROR: code %i\nmsg: %s\n", error, description);
}

static void glfw_window_size_callback(GLFWwindow *window, int width,
                                      int height) {
  gl_window_width = width;
  gl_window_height = height;
}

int main(int argc, const char const *argv[]) {
  // register the error call back function
  glfwSetErrorCallback(glfw_error_callback);

  // start GL context and os window using GLFW helper library
  if (!glfwInit()) {
    printf("ERROR: could not start GLFW3\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR_VERSION);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR_VERSION);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // GL anti-alias
  glfwWindowHint(GLFW_SAMPLES, ANTI_ALIAS_LEVEL);

  GLFWwindow *window = glfwCreateWindow(gl_window_width, gl_window_height,
                                        "Open GL Realtime Viewer", NULL, NULL);

  // use moniotr information to create the gl window {

  // get monitor information for gl window {
  // GLFWmonitor *mon = glfwGetPrimaryMonitor();
  // const GLFWvidmode *vmode = glfwGetVideoMode(mon);
  // }

  // GLFWwindow *window =
  //     glfwCreateWindow(vmode->width, vmode->height, "Hello GL", NULL, NULL);
  // }

  if (!window) {
    printf("ERROR: Could not create the GL window with GLFW3\n");
    glfwTerminate();
    return 1;
  }

  glfwSetWindowSizeCallback(window, glfw_window_size_callback);

  glfwMakeContextCurrent(window);

  // start GLEW extensions handler
  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  printf("OpenGL Renderer: %s\n", renderer);
  printf("OpenGL Version: %s\n", version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // MAIN OPENGL PROGRAM--------------------------------------------------------

  // loading obj using parser {
  obj *model = load_obj(OBJ_FILE);
  __ASSERT(model);
  __ASSERT(model->vp);
  __ASSERT(model->vn);
  __ASSERT(model->vt);
  __ASSERT(model->vtan);

  // generate vertex attribute object to store buffer
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  generate_vbo(&vao, model->point_counts, 3, model->vp, 0);
  generate_vbo(&vao, model->point_counts, 3, model->vn, 1);
  generate_vbo(&vao, model->point_counts, 2, model->vt, 2);
  generate_vbo(&vao, model->point_counts, 4, model->vtan, 3);

  // // to modified the vertex buffer objects, we need to bind it first
  // glBindBuffer(GL_ARRAY_BUFFER, vbo_p);
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  // glBindBuffer(GL_ARRAY_BUFFER, vbo_n);
  // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  // // texture coordinates has dimention of 2
  // glBindBuffer(GL_ARRAY_BUFFER, vbo_st);
  // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  //
  // // glEnableVertexAttribArray() only affects current bound vertex array
  // object
  // // in this case the vao
  // glEnableVertexAttribArray(0);
  // glEnableVertexAttribArray(1);
  // glEnableVertexAttribArray(2);

  // read shader from separate file
  const char *vertex_shader = readShader(VERTEX_SHADER_FILE);
  const char *fragment_shader = readShader(FRAGMENT_SHADER_FILE);

  // // create gl shader and compile them
  // GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  // glShaderSource(vs, 1, &vertex_shader, NULL);
  //
  // // original GL compile shader function call
  // // glCompileShader(vs);
  //
  // // wrapper function with gl shader compile and error check
  // __CompileShader(vs);
  //
  // GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  // glShaderSource(fs, 1, &fragment_shader, NULL);
  //
  // // original GL compile shader function call
  // // glCompileShader(fs);
  //
  // // wrapper function with gl shader compile and error check
  // __CompileShader(fs);

  // create gl shader program
  GLuint shader_program = glCreateProgram();
  generate_shader(&shader_program, vertex_shader, GL_VERTEX_SHADER);
  generate_shader(&shader_program, fragment_shader, GL_FRAGMENT_SHADER);
  // glAttachShader(shader_program, fs);
  // glAttachShader(shader_program, vs);

  // for compatible with opengl 3.2, we cannot use the "layout" keyword in the
  // shader.  Instead, we should add folowing codes {
  // glBindAttribLocation(shader_program, 0, "vertex_position");
  // glBindAttribLocation(shader_program, 1, "vertex_color");
  // }

  // original GL programming linking call
  // glLinkProgram(shader_program);

  // wrapper function with gl program link and error check
  __LinkProgram(shader_program);

  // import and generate texture {

  // Note:
  // texture binding should be excuted after shader program linking
  if (!load_texture(OBJ_DIFFUSE_MAP, &shader_program, GL_TEXTURE0,
                    "diff_tex")) {
    printf("diffuse texture loading failed!\n");
  }
  if (!load_texture(OBJ_SPECULAR_MAP, &shader_program, GL_TEXTURE1,
                    "spec_tex")) {
    printf("specular texture loading failed!\n");
  }
  if (!load_texture(OBJ_NORMAL_MAP, &shader_program, GL_TEXTURE2,
                    "normal_tex")) {
    printf("normal texture loading failed!\n");
  }
  // }

  // set up uniform attributes for moving 3d model {
  // int model_mat_loc = glGetUniformLocation(shader_program, "model_mat");
  // glUseProgram(shader_program);
  // glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_mat);
  // }

  // parameters for moving triangle {
  // float move_speed = 1.0f;
  // float last_position = 0.0f;
  // }

  // parameters for camera movement {
  // 1 unit per seconds
  float cam_speed = 1.0f;
  // 10 degress per seconds
  float cam_yaw_speed = 60.0f;

  float cam_pos[] = {0.0f, 0.0, 3.0f};
  float cam_yaw = 0.0f;

  mat4 camT = translate(identity_mat4(),
                        vec3_new(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
  mat4 camR = rotate_y_deg(identity_mat4(), -cam_yaw);

  mat4 view_mat = mat4_mul_mat4(camR, camT);
  mat4 proj_mat = perspective(
      67.0f, (float)gl_window_width / (float)gl_window_height, 0.1f, 100.0f);

  int view_mat_loc = glGetUniformLocation(shader_program, "view_mat");
  glUseProgram(shader_program);
  glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, view_mat);

  int proj_mat_loc = glGetUniformLocation(shader_program, "proj_mat");
  glUseProgram(shader_program);
  glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, proj_mat);

  // }

  // // setup uniform variable for texture {
  // glActiveTexture(GL_TEXTURE1);
  // int slot_index = get_texture_slot_int(GL_TEXTURE1);
  // int diff_tex_loc = glGetUniformLocation(shader_program, "spec_tex");
  // glUseProgram(shader_program);
  // glUniform1i(diff_tex_loc, slot_index);
  // // }

  // enable back face culling {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // GL_CCW for counter clock-wise vertex winding order
  glFrontFace(GL_CCW);
  // }

  // drawing loop
  while (!glfwWindowShouldClose(window)) {
    // code for time calculation {
    static double previous_seconds = 0.0f;
    double current_seconds = glfwGetTime();
    double elapsed_seconds = current_seconds - previous_seconds;
    previous_seconds = current_seconds;
    // }

    // code for moving triangle with uniform matrix {
    // // moving_triangle(shader_program, t_matrix, t_matrix_loc);
    //
    // if (fabs(last_position) > 1.0f) {
    //   move_speed = -move_speed;
    // }
    //
    // t_matrix[12] = (elapsed_seconds * move_speed) + last_position;
    // last_position = t_matrix[12];
    // glUseProgram(shader_program);
    // glUniformMatrix4fv(t_matrix_loc, 1, GL_FALSE, t_matrix);
    // }

    // code for camera control {
    if (camera_control(window, cam_pos, &cam_yaw, cam_speed, cam_yaw_speed,
                       elapsed_seconds)) {
      mat_free(camT);
      mat_free(camR);

      camT = translate(identity_mat4(),
                       vec3_new(-cam_pos[0], -cam_pos[1], -cam_pos[2]));

      // using quaternion to calculate rotation matrix {
      // multiply quaternion to combine rotation
      versor rot_y = quat_from_axis_deg(cam_yaw, 0.0f, 1.0f, 0.0f);
      camR = quat_to_mat4(rot_y);
      versor_free(rot_y);
      // }

      // camR = rotate_y_deg(identity_mat4(), -cam_yaw);
      mat4 new_mat = mat4_mul_mat4(camR, camT);
      mat4_components_assign(view_mat, new_mat);
      mat_free(new_mat);

      glUseProgram(shader_program);
      glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, view_mat);
    }
    // }

    update_fps_counter(window);
    // clear the gl buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // resize the gl viewport
    glViewport(0, 0, gl_window_width, gl_window_height);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, model->point_counts);

    // update other events like input handling
    glfwPollEvents();
    // put buffer onto the display
    glfwSwapBuffers(window);

    // escape window when pressing esc
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }
  }

  // END OPENGL PROGRAM

  // close GL context and any other GLFW resourse
  glfwTerminate();
  return 0;
}

void generate_shader(GLuint *shader_program, const char *shader_file,
                     GLenum shader_type) {
  GLuint shader;
  switch (shader_type) {
  case GL_VERTEX_SHADER: {
    shader = glCreateShader(GL_VERTEX_SHADER);
    break;
  }
  case GL_FRAGMENT_SHADER: {
    shader = glCreateShader(GL_FRAGMENT_SHADER);
    break;
  }
  }
  glShaderSource(shader, 1, &shader_file, NULL);

  // original GL compile shader function call
  // glCompileShader(fs);

  // wrapper function with gl shader compile and error check
  __CompileShader(shader);
  glAttachShader(*shader_program, shader);
}

void generate_vbo(GLuint *vao, int point_count, int vector_size,
                  float *data_array, int loc) {
  // generate vbo
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * point_count * vector_size,
               data_array, GL_STATIC_DRAW);

  // bind to the specific vao
  glBindVertexArray(*vao);
  // to modified the vertex buffer objects, we need to bind the specific vbo
  // first
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // watch out the index and the vector size
  glVertexAttribPointer(loc, vector_size, GL_FLOAT, GL_FALSE, 0, NULL);
  // enable the requested index to be used
  glEnableVertexAttribArray(loc);
}

bool load_texture(const char *texture_file, GLuint *shader_program,
                  GLenum texture_slot, const char *texture_name) {
  int x, y, n;
  int force_channels = 4;
  unsigned char *image_data =
      stbi_load(texture_file, &x, &y, &n, force_channels);

  if (!image_data) {
    fprintf(stderr, "ERROR: counld not load %s\n", texture_file);
    return false;
  }

  // check if the texture dimension is a power of 2
  if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
    fprintf(stderr,
            "WARNING: the dimensions of the texture %s is not power of 2\n",
            texture_file);
  }

  // flip the image data upside down because OpenGL expects that the 0 on the y
  // axis to be at the bottom of the texture, but the image usually have y axis
  // 0 at the top
  int width_in_bytes = x * 4;
  unsigned char *top = NULL;
  unsigned char *bottom = NULL;
  unsigned char temp = 0;
  int half_height = y / 2;

  for (int row = 0; row < half_height; row++) {
    top = image_data + row * width_in_bytes;
    bottom = image_data + ((y - row - 1) * width_in_bytes);
    for (int col = 0; col < width_in_bytes; col++) {
      temp = *top;
      *top = *bottom;
      *bottom = temp;
      top++;
      bottom++;
    }
  }

  GLuint tex = 0;
  glGenTextures(1, &tex);
  // active the first OpenGL texture slot
  glActiveTexture(texture_slot);
  int slot_index = get_texture_slot_int(texture_slot);
  printf("texture: %s, slot index: %d\n", texture_name, slot_index);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               image_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glActiveTexture(texture_slot);
  int tex_loc = glGetUniformLocation(*shader_program, texture_name);
  glUseProgram(*shader_program);
  glUniform1i(tex_loc, slot_index);
  return true;
}

bool camera_control(GLFWwindow *window, float *cam_pos, float *cam_yaw,
                    float cam_speed, float cam_yaw_speed,
                    double elapsed_seconds) {
  bool cam_moved = false;
  if (glfwGetKey(window, GLFW_KEY_H)) {
    cam_pos[0] = 0.0f;
    cam_pos[1] = 0.0f;
    cam_pos[2] = 3.0f;
    *cam_yaw = 0.0f;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_A)) {
    cam_pos[0] -= cam_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_D)) {
    cam_pos[0] += cam_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_W)) {
    cam_pos[2] -= cam_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_S)) {
    cam_pos[2] += cam_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_Q)) {
    cam_pos[1] += cam_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_E)) {
    cam_pos[1] -= cam_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_C)) {
    *cam_yaw += cam_yaw_speed * elapsed_seconds;
    cam_moved = true;
  }
  if (glfwGetKey(window, GLFW_KEY_Z)) {
    *cam_yaw -= cam_yaw_speed * elapsed_seconds;
    cam_moved = true;
  }
  return cam_moved;
}

// void moving_triangle(GLuint program, float *matrix, int matrix_loc) {
//   printf("%f\n", matrix[12]);
//   float move_speed = 1.0f;
//   float last_position = 0.0f;
//
//   static double previous_seconds = 0.0f;
//   double current_seconds = glfwGetTime();
//   double elapsed_seconds = current_seconds - previous_seconds;
//   previous_seconds = current_seconds;
//
//   if (fabs(last_position) > 1.0f) {
//     move_speed = -move_speed;
//   }
//
//   matrix[12] = (elapsed_seconds * move_speed) + last_position;
//   last_position = matrix[12];
//   glUseProgram(program);
//   glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
// }
