#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glib-2.0/glib.h>

//#include <cmockery/pbc.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../general/src/debug_macro.h"
#include "../../general/src/linear_math.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION

#include "../../third_party/nuklear/nuklear.h"
////
#include "../../third_party/nuklear/demo/glfw_opengl3/nuklear_glfw_gl3.h"

#include "obj_model.h"
#include "shader.h"

#define ANTI_ALIAS_LEVEL 8

#define MAX_VERTEX_BUFFER 256 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

//#define SHADER_CONTROL_MARGIN 10

//#define STANDARD_SHADER_VERTEX "src/shader/vertex_shader.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/fragment_shader.frag"

//#define STANDARD_SHADER_VERTEX "src/shader/standard.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/standard.frag"

//#define STANDARD_SHADER_VERTEX "src/shader/constant.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/constant.frag"

#define STANDARD_SHADER_VERTEX "src/shader/reflective.vert"
#define STANDARD_SHADER_FRAGMENT "src/shader/reflective.frag"

//#define STANDARD_SHADER_VERTEX "src/shader/reference_shader.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/reference_shader.frag"

//#define TEST_MODEL "geo/monkey/test.obj"

//#define TEST_MODEL "geo/05.obj"
//#define TEST_MODEL "geo/05_01.obj"
#define TEST_MODEL "geo/sphere_base_02.obj"
//#define TEST_MODEL "geo/07.obj"
//
//#define TEST_MODEL_COLOR "geo/05_color.jpg"
//#define TEST_MODEL_SPEC "geo/05_reflect.jpg"
//#define TEST_MODEL_NORMAL "geo/05_normalbump.jpg"

#define TEST_MODEL_COLOR "geo/uv_checker.png"
#define TEST_MODEL_SPEC "geo/uv_checker.png"
#define TEST_MODEL_NORMAL "geo/rocky_tile_nor.png"

#define TEST_CUBE_MAP_NEG_Z "cube_map/Yokohama/negz.jpg"
#define TEST_CUBE_MAP_POS_Z "cube_map/Yokohama/posz.jpg"
#define TEST_CUBE_MAP_NEG_Y "cube_map/Yokohama/negy.jpg"
#define TEST_CUBE_MAP_POS_Y "cube_map/Yokohama/posy.jpg"
#define TEST_CUBE_MAP_NEG_X "cube_map/Yokohama/negx.jpg"
#define TEST_CUBE_MAP_POS_X "cube_map/Yokohama/posx.jpg"

#define SEPARATOR "---------------------------------------------"

static int glfwWindowWidth = 1600;
static int glfwWindowHeight = 900;

static void glfwErrorCallback(int error, const char *description) {
  printf("GLFW ERROR: code %i\nmsg: %s\n", error, description);
  exit(EXIT_FAILURE);
}

static void glfwWindowSizeCallback(GLFWwindow *window, int width, int height) {
  glfwWindowWidth = width;
  glfwWindowHeight = height;
}

static void updateFpsCounter(GLFWwindow *window) {
  static double previousSeconds = 0.0f;
  static int frameCount = 0;

  double currentSeconds = glfwGetTime();
  double elapsedSeconds = currentSeconds - previousSeconds;

  if (elapsedSeconds > 0.5) {
    previousSeconds = currentSeconds;
    char tmp[128];

    double fps = (double)frameCount / elapsedSeconds;
    sprintf(tmp, "OpenGL Model Viewer @ FPS: %.4f", fps);
    glfwSetWindowTitle(window, tmp);
    frameCount = 0;
  }

  frameCount++;
}

gboolean camera_control(GLFWwindow *window, float *cam_pos, float *cam_yaw,
                        double cam_speed, double cam_yaw_speed,
                        double elapsed_seconds) {
  // gboolean cam_moved = FALSE;

  if (glfwGetKey(window, GLFW_KEY_H)) {
    cam_pos[0] = 0.0f;
    cam_pos[1] = 0.0f;
    cam_pos[2] = 2.5f;
    *cam_yaw = -45.0f;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_A)) {
    cam_pos[0] -= cam_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_D)) {
    cam_pos[0] += cam_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_W)) {
    cam_pos[2] -= cam_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_S)) {
    cam_pos[2] += cam_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_Q)) {
    cam_pos[1] += cam_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_E)) {
    cam_pos[1] -= cam_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  if (glfwGetKey(window, GLFW_KEY_C)) {
    *cam_yaw += cam_yaw_speed * elapsed_seconds;
    goto return_ture;
    // cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_Z)) {
    *cam_yaw -= cam_yaw_speed * elapsed_seconds;
    // cam_moved = TRUE;
    goto return_ture;
  }
  // return cam_moved;

  return FALSE;

return_ture:
  return TRUE;
}

static char *modelFile = NULL;
static char *diffuseFile = NULL;
static char *reflectFile = NULL;
static char *normalFile = NULL;

static struct Shader *modelShader = NULL;
static struct ObjModel *model = NULL;

static struct nk_color diffuseColor;
static struct nk_color reflectionColor;
// static float reflectionColor[3] = {1.0f, 1.0f, 1.0f};

static int useDiffuseMap = 0;
static int useReflectionMap = 0;
static int useNormalMap = 0;

static float normalIntensity = 0.0f;

static void fileDropCallBack(GLFWwindow *window, int numFiles,
                             const char **filePath) {
  if (numFiles > 1) {
    printf("Only accept one file at a time\n");
    return;
  }

  double x, y;

  glfwGetCursorPos(window, &x, &y);

  if (x > 30 && x < 250) {
    if (y > 92 && y < 122) {
      free(modelFile);

      printf("model file: %s\n", filePath[0]);
      modelFile = pathGetBase(filePath[0]);

      objModelFree(model);
      model = objModelNew((char *)filePath[0], modelShader);

      return;

    } else if (y > 296 && y < 326 && useDiffuseMap) {
      free(diffuseFile);

      printf("diffuse map: %s\n", filePath[0]);
      diffuseFile = pathGetBase(filePath[0]);

      shaderSetDiffuseTexture(modelShader, (char *)filePath[0], GL_TEXTURE6);

      return;
    } else if (y > 500 && y < 530 && useReflectionMap) {
      free(reflectFile);

      printf("reflection map: %s\n", filePath[0]);
      reflectFile = pathGetBase(filePath[0]);

      shaderSetSpecularTexture(modelShader, (char *)filePath[0], GL_TEXTURE7);

      return;
    } else if (y > 636 && y < 666 && useNormalMap) {
      free(normalFile);

      printf("normal map: %s\n", filePath[0]);
      normalFile = pathGetBase(filePath[0]);

      shaderSetNormalTexture(modelShader, (char *)filePath[0], GL_TEXTURE8);

      return;
    }
  }

  // printf("file path: %s\n", filePath[0]);
}

int main(int argc, char const *argv[]) {
  // register the error call back function
  glfwSetErrorCallback(glfwErrorCallback);

  // start GL context and os window using GLFW helper library
  if (!glfwInit()) {
    printf("ERROR: could not start GLFW3\n");
    exit(EXIT_FAILURE);
  }

  // request opengl 4.3 in mesa
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // GL anti-alias
  glfwWindowHint(GLFW_SAMPLES, ANTI_ALIAS_LEVEL);

  // create gl window
  GLFWwindow *window = glfwCreateWindow(glfwWindowWidth, glfwWindowHeight,
                                        "OpenGL Model Viewer", NULL, NULL);

  if (!window) {
    printf("ERROR: Could not create the GL window with GLFW3\n");
    glfwTerminate();
    return 1;
  }

  glfwSetWindowSizeCallback(window, glfwWindowSizeCallback);

  glfwSetDropCallback(window, fileDropCallBack);

  // get information about the primary monitor
  GLFWmonitor *mon = glfwGetPrimaryMonitor();
  const GLFWvidmode *vmode = glfwGetVideoMode(mon);

  // set window position to the center of the monitor
  glfwSetWindowPos(window, (vmode->width / 2) - (glfwWindowWidth / 2),
                   (vmode->height / 2) - (glfwWindowHeight / 2));

  glfwMakeContextCurrent(window);

  // start GLEW extensions handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  printf("OpenGL Renderer: %s\n", renderer);
  printf("OpenGL Version: %s\n", version);

  // only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // GL_CCW for counter clock-wise vertex winding order
  glFrontFace(GL_CCW);

  // NUKLEAR UI--------------------------------------------------------

  struct nk_context *ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

  struct nk_font_atlas *atlas;

  nk_glfw3_font_stash_begin(&atlas);

  struct nk_font *droid = nk_font_atlas_add_from_file(
      atlas, "/usr/share/fonts/liberation/LiberationSans-Regular.ttf", 16, 0);

  nk_glfw3_font_stash_end();

  // nk_style_load_all_cursors(ctx, atlas->cursors);
  nk_style_set_font(ctx, &droid->handle);

  // diffuseColor = nk_rgb(255, 255, 255);
  diffuseColor = nk_rgb(0, 0, 0);
  reflectionColor = nk_rgb(255, 255, 255);

  // MAIN OPENGL
  // PROGRAM--------------------------------------------------------

  // IMPORT MODEL AND CREATE
  // SHADER-------------------------------------------------------------------------

  // struct Shader *modelShader =
  //    shaderNew(STANDARD_SHADER_VERTEX, STANDARD_SHADER_FRAGMENT);

  modelShader = shaderNew(STANDARD_SHADER_VERTEX, STANDARD_SHADER_FRAGMENT);

  shaderSetCubeMap(modelShader, TEST_CUBE_MAP_NEG_Z, TEST_CUBE_MAP_POS_Z,
                   TEST_CUBE_MAP_POS_Y, TEST_CUBE_MAP_NEG_Y,
                   TEST_CUBE_MAP_POS_X, TEST_CUBE_MAP_NEG_X, GL_TEXTURE9);

  shaderSetDiffuseTexture(modelShader, TEST_MODEL_COLOR, GL_TEXTURE6);
  shaderSetSpecularTexture(modelShader, TEST_MODEL_SPEC, GL_TEXTURE7);
  shaderSetNormalTexture(modelShader, TEST_MODEL_NORMAL, GL_TEXTURE8);

  // struct ObjModel *model = objModelNew(TEST_MODEL, modelShader);
  model = objModelNew(TEST_MODEL, modelShader);

  // IMPORT MODEL AND CREATE
  // SHADER-------------------------------------------------------------------------

  float cam_speed = 1.0f;

  // 10 degress per seconds
  float cam_yaw_speed = 60.0f;

  float cam_pos[] = {0.0f, 0.0, 2.5f};
  float cam_yaw = -45.0f;

  mat4 i = identityMat4();
  vec3 camDir = vec3New(-cam_pos[0], -cam_pos[1], -cam_pos[2]);

  mat4 camT = translate(i, camDir);

  versor rot_y = quatFromAxisDeg(cam_yaw, 0.0f, 1.0f, 0.0f);
  mat4 camR = quatToMat4(rot_y);
  // mat4 camR = rotateYdeg(i, -cam_yaw);

  mat4 view_mat = mat4MulMat4(camR, camT);

  vecFree(camDir);
  matFree(i);
  matFree(camT);
  matFree(camR);

  mat4 proj_mat = perspective(
      67.0f, (double)glfwWindowWidth / (double)glfwWindowHeight, 0.1f, 100.0f);

  int view_mat_loc =
      glGetUniformLocation(modelShader->shaderProgram, "view_mat");

  glUseProgram(modelShader->shaderProgram);

  glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, mat4GetData(view_mat));

  int proj_mat_loc =
      glGetUniformLocation(modelShader->shaderProgram, "proj_mat");

  glUseProgram(modelShader->shaderProgram);

  glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, mat4GetData(proj_mat));

  // drawing loop
  while (!glfwWindowShouldClose(window)) {
    static double previous_seconds = 0.0f;
    double current_seconds = glfwGetTime();
    double elapsed_seconds = current_seconds - previous_seconds;
    previous_seconds = current_seconds;

    if (camera_control(window, cam_pos, &cam_yaw, cam_speed, cam_yaw_speed,
                       elapsed_seconds)) {
      mat4 i = identityMat4();
      vec3 camDir = vec3New(-cam_pos[0], -cam_pos[1], -cam_pos[2]);

      mat4 camT = translate(i, camDir);

      // using quaternion to calculate rotation matrix
      // multiply quaternion to combine rotation
      versor rot_y = quatFromAxisDeg(cam_yaw, 0.0f, 1.0f, 0.0f);
      camR = quatToMat4(rot_y);
      versorFree(rot_y);

      mat4 new_mat = mat4MulMat4(camR, camT);
      mat4ComponentsAssign(view_mat, new_mat);

      vecFree(camDir);
      matFree(i);
      matFree(camT);
      matFree(camR);
      matFree(new_mat);

      glUseProgram(modelShader->shaderProgram);
      glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, mat4GetData(view_mat));
    }

    updateFpsCounter(window);

    glfwPollEvents();

    // NUKLEAR UI--------------------------------------------------------

    nk_glfw3_new_frame();

    struct nk_panel layout;

    if (nk_begin(ctx, &layout, "Shader Control", nk_rect(20, 20, 250, 850),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                     NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
      nk_layout_row_dynamic(ctx, 30, 1);

      nk_label(ctx, "OBJ Model: ", NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, 30, 1);

      if (nk_button_label(ctx, modelFile == NULL ? "sphere.obj" : modelFile)) {
        printf("Model pressed\n");
      }

      nk_label(ctx, SEPARATOR, NK_TEXT_LEFT);

      // nk_layout_row_dynamic(ctx, 30, 1);

      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, "Diffuse Color:", NK_TEXT_LEFT);

      struct nk_panel diffuseColorCombo;
      nk_layout_row_dynamic(ctx, 30, 1);

      if (nk_combo_begin_color(ctx, &diffuseColorCombo, diffuseColor,
                               nk_vec2(nk_widget_width(ctx), 400))) {
        nk_layout_row_dynamic(ctx, 120, 1);

        diffuseColor = nk_color_picker(ctx, diffuseColor, NK_RGB);

        nk_layout_row_dynamic(ctx, 30, 1);

        diffuseColor.r =
            (nk_byte)nk_propertyi(ctx, "#R:", 0, diffuseColor.r, 255, 1, 1);

        diffuseColor.g =
            (nk_byte)nk_propertyi(ctx, "#G:", 0, diffuseColor.g, 255, 1, 1);

        diffuseColor.b =
            (nk_byte)nk_propertyi(ctx, "#B:", 0, diffuseColor.b, 255, 1, 1);

        nk_combo_end(ctx);

        shaderSetDiffuseColor(modelShader, (float)diffuseColor.r / 255.0f,
                              (float)diffuseColor.g / 255.0f,
                              (float)diffuseColor.b / 255.0f);
      }

      nk_layout_row_dynamic(ctx, 30, 1);
      if (nk_checkbox_label(ctx, "Use Diffuse Map", &useDiffuseMap)) {
        shaderSetUseDiffuseMap(modelShader, (int)useDiffuseMap);
      }

      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, "Diffuse Map: ", NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, 30, 1);
      if (nk_button_label(ctx, diffuseFile == NULL ? "EMPTY" : diffuseFile)) {
        printf("Diffuse Map pressed\n");
      }

      nk_label(ctx, SEPARATOR, NK_TEXT_LEFT);

      //      static int glossiness = 20;
      //      nk_layout_row_dynamic(ctx, 25, 1);
      //      nk_property_int(ctx, "Glossiness:", 0, &glossiness, 100, 10, 1);

      // nk_layout_row_dynamic(ctx, 30, 1);

      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, "Reflection Color:", NK_TEXT_LEFT);

      struct nk_panel reflectionColorCombo;
      nk_layout_row_dynamic(ctx, 30, 1);

      if (nk_combo_begin_color(ctx, &reflectionColorCombo, reflectionColor,
                               nk_vec2(nk_widget_width(ctx), 400))) {
        nk_layout_row_dynamic(ctx, 120, 1);

        reflectionColor = nk_color_picker(ctx, reflectionColor, NK_RGB);

        nk_layout_row_dynamic(ctx, 30, 1);

        reflectionColor.r =
            (int)nk_propertyi(ctx, "#R:", 0, reflectionColor.r, 255, 1, 1);

        reflectionColor.g =
            (int)nk_propertyi(ctx, "#G:", 0, reflectionColor.g, 255, 1, 1);

        reflectionColor.b =
            (int)nk_propertyi(ctx, "#B:", 0, reflectionColor.b, 255, 1, 1);

        nk_combo_end(ctx);

        shaderSetSpecularColor(modelShader, (float)reflectionColor.r / 255.0f,
                               (float)reflectionColor.g / 255.0f,
                               (float)reflectionColor.b / 255.0f);
      }

      nk_layout_row_dynamic(ctx, 30, 1);
      if (nk_checkbox_label(ctx, "Use Reflection Map", &useReflectionMap)) {
        shaderSetUseSpecularMap(modelShader, (int)useReflectionMap);
      }

      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, "Reflection Map: ", NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, 30, 1);
      if (nk_button_label(ctx, reflectFile == NULL ? "EMPTY" : reflectFile)) {
        printf("Reflection Map pressed\n");
      }

      nk_label(ctx, SEPARATOR, NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, 30, 1);
      if (nk_checkbox_label(ctx, "Use Normal Map", &useNormalMap)) {
        shaderSetUseNormalMap(modelShader, (int)useNormalMap);
      }

      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, "Normal Map: ", NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, 30, 1);
      if (nk_button_label(ctx, normalFile == NULL ? "EMPTY" : normalFile)) {
        printf("Normal Map pressed\n");
      }

      nk_layout_row_dynamic(ctx, 30, 1);
      static char normalIntensityRe[28] = "Normal Map Intensity:  1.00";
      nk_label(ctx, normalIntensityRe, NK_TEXT_LEFT);
      if (nk_slider_float(ctx, 0, &normalIntensity, 2, 0.01)) {
        sprintf(normalIntensityRe, "Normal Map Intensity:  %.2f",
                normalIntensity);

        shaderSetNormalIntensity(modelShader, normalIntensity);
      }

      // nk_label(ctx, normalIntensityRe, NK_TEXT_CENTERED);

      //      struct nk_panel combo;
      //      nk_layout_row_dynamic(ctx, 20, 1);
      //
      //      nk_label(ctx, "Reflection:", NK_TEXT_LEFT);
      //
      //      nk_layout_row_dynamic(ctx, 25, 1);
      //
      //      if (nk_combo_begin_color(ctx, &combo, background,
      //                               nk_vec2(nk_widget_width(ctx), 400))) {
      //        nk_layout_row_dynamic(ctx, 120, 1);
      //
      //        background = nk_color_picker(ctx, background, NK_RGBA);
      //
      //        nk_layout_row_dynamic(ctx, 25, 1);
      //
      //        background.r =
      //            (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1,
      //            1);
      //
      //        background.g =
      //            (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1,
      //            1);
      //
      //        background.b =
      //            (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1,
      //            1);
      //
      //        background.a =
      //            (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1,
      //            1);
      //
      //        nk_combo_end(ctx);
      //      }

      //      static int glossiness = 20;
      //      nk_layout_row_dynamic(ctx, 25, 1);
      //      nk_property_int(ctx, "Glossiness:", 0, &glossiness, 100, 10, 1);
    }

    nk_end(ctx);

    //    double x, y;
    //    glfwGetCursorPos(window, &x, &y);
    //
    //    printf("X: %f\n", x);
    //    printf("Y: %f\n", y);

    // NUKLEAR UI--------------------------------------------------------

    glfwMakeContextCurrent(window);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glFrontFace(GL_CCW);

    // clear the gl buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // resize the gl viewport
    glViewport(0, 0, glfwWindowWidth, glfwWindowHeight);

    glUseProgram(modelShader->shaderProgram);

    glBindVertexArray(model->VAO);
    glDrawArrays(GL_TRIANGLES, 0, model->model->point_counts);

    // glDrawArrays(GL_LINES, 0, model->model->point_counts);
    // glDrawArrays(GL_POINTS, 0, model->model->point_counts);

    // NUKLEAR UI--------------------------------------------------------

    nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    // NUKLEAR UI--------------------------------------------------------

    // update other events like input handling

    // put buffer onto the display
    glfwSwapBuffers(window);

    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }
  }

  // END OPENGL PROGRAM

  // NUKLEAR UI--------------------------------------------------------

  nk_glfw3_shutdown();

  // NUKLEAR UI--------------------------------------------------------

  // close GL context and any other GLFW resourse
  glfwTerminate();
  return 0;
}
