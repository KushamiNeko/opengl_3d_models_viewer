#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glib-2.0/glib.h>

//#include <cmockery/pbc.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../general/src/debug_macro.h"
#include "../../general/src/linear_math.h"

#include "obj_model.h"
#include "shader.h"

#define ANTI_ALIAS_LEVEL 4

//#define STANDARD_SHADER_VERTEX "src/shader/vertex_shader.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/fragment_shader.frag"

//#define STANDARD_SHADER_VERTEX "src/shader/standard.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/standard.frag"

//#define STANDARD_SHADER_VERTEX "src/shader/constant.vert"
//#define STANDARD_SHADER_FRAGMENT "src/shader/constant.frag"

#define STANDARD_SHADER_VERTEX "src/shader/reflective.vert"
#define STANDARD_SHADER_FRAGMENT "src/shader/reflective.frag"

//#define TEST_MODEL "geo/monkey/test.obj"

//#define TEST_MODEL "geo/05.obj"
#define TEST_MODEL "geo/05_01.obj"
//#define TEST_MODEL "geo/07.obj"
#define TEST_MODEL_COLOR "geo/05_color.jpg"
#define TEST_MODEL_SPEC "geo/05_reflect.jpg"
#define TEST_MODEL_NORMAL "geo/05_normalbump.jpg"

#define TEST_CUBE_MAP_NEG_Z "cube_map/Yokohama/negz.jpg"
#define TEST_CUBE_MAP_POS_Z "cube_map/Yokohama/posz.jpg"
#define TEST_CUBE_MAP_NEG_Y "cube_map/Yokohama/negy.jpg"
#define TEST_CUBE_MAP_POS_Y "cube_map/Yokohama/posy.jpg"
#define TEST_CUBE_MAP_NEG_X "cube_map/Yokohama/negx.jpg"
#define TEST_CUBE_MAP_POS_X "cube_map/Yokohama/posx.jpg"

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
  gboolean cam_moved = FALSE;
  if (glfwGetKey(window, GLFW_KEY_H)) {
    cam_pos[0] = 0.0f;
    cam_pos[1] = 0.0f;
    cam_pos[2] = 3.0f;
    *cam_yaw = 0.0f;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_A)) {
    cam_pos[0] -= cam_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_D)) {
    cam_pos[0] += cam_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_W)) {
    cam_pos[2] -= cam_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_S)) {
    cam_pos[2] += cam_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_Q)) {
    cam_pos[1] += cam_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_E)) {
    cam_pos[1] -= cam_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_C)) {
    *cam_yaw += cam_yaw_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  if (glfwGetKey(window, GLFW_KEY_Z)) {
    *cam_yaw -= cam_yaw_speed * elapsed_seconds;
    cam_moved = TRUE;
  }
  return cam_moved;
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

  // get information about the primary monitor
  GLFWmonitor *mon = glfwGetPrimaryMonitor();
  const GLFWvidmode *vmode = glfwGetVideoMode(mon);

  // set window position to the center of the monitor
  glfwSetWindowPos(window, (vmode->width / 2) - (glfwWindowWidth / 2),
                   (vmode->height / 2) - (glfwWindowHeight / 2));

  glfwSetWindowSizeCallback(window, glfwWindowSizeCallback);
  if (!window) {
    printf("ERROR: Could not create the GL window with GLFW3\n");
    glfwTerminate();
    return 1;
  }

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

  // MAIN OPENGL PROGRAM--------------------------------------------------------

  struct Shader *modelShader =
      shaderNew(STANDARD_SHADER_VERTEX, STANDARD_SHADER_FRAGMENT);

  shaderSetCubeMap(modelShader, TEST_CUBE_MAP_NEG_Z, TEST_CUBE_MAP_POS_Z,
                   TEST_CUBE_MAP_POS_Y, TEST_CUBE_MAP_NEG_Y,
                   TEST_CUBE_MAP_POS_X, TEST_CUBE_MAP_NEG_X, GL_TEXTURE3);

  shaderSetDiffuseTexture(modelShader, TEST_MODEL_COLOR, GL_TEXTURE0);
  shaderSetSpecularTexture(modelShader, TEST_MODEL_SPEC, GL_TEXTURE1);
  shaderSetNormalTexture(modelShader, TEST_MODEL_NORMAL, GL_TEXTURE2);

  struct ObjModel *model = objModelNew(TEST_MODEL, modelShader);

  float cam_speed = 1.0f;

  // 10 degress per seconds
  float cam_yaw_speed = 60.0f;

  float cam_pos[] = {0.0f, 0.0, 2.5f};
  float cam_yaw = 0.0f;

  mat4 i = identityMat4();
  vec3 camDir = vec3New(-cam_pos[0], -cam_pos[1], -cam_pos[2]);

  mat4 camT = translate(i, camDir);

  mat4 camR = rotateYdeg(i, -cam_yaw);

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

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // GL_CCW for counter clock-wise vertex winding order
  glFrontFace(GL_CCW);

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

    // clear the gl buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // resize the gl viewport
    glViewport(0, 0, glfwWindowWidth, glfwWindowHeight);

    glUseProgram(modelShader->shaderProgram);
    glBindVertexArray(model->VAO);
    glDrawArrays(GL_TRIANGLES, 0, model->model->point_counts);
    // glDrawArrays(GL_LINES, 0, model->model->point_counts);
    // glDrawArrays(GL_POINTS, 0, model->model->point_counts);

    // update other events like input handling
    glfwPollEvents();
    // put buffer onto the display
    glfwSwapBuffers(window);

    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }
  }

  // END OPENGL PROGRAM

  // close GL context and any other GLFW resourse
  glfwTerminate();
  return 0;
}
