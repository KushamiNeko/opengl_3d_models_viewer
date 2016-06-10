#include "../header/obj_model.h"

static GLuint constShaderProgram(const char *vertFile, const char *fragFile) {
  const char *vertexShader = readShader(vertFile);
  const char *fragmentShader = readShader(fragFile);

  GLuint shaderProgram = glCreateProgram();
  generateShader(&shaderProgram, vertexShader, GL_VERTEX_SHADER);
  generateShader(&shaderProgram, fragmentShader, GL_FRAGMENT_SHADER);

  __LinkProgram(shaderProgram);

  return shaderProgram;
}

struct ObjModel *objModelNew(const char *objFile, const char *vertexShader,
                             const char *fragmentShader) {
  struct ObjModel *re = defenseCalloc(1, sizeof(struct ObjModel));

  re->shaderProgram = constShaderProgram(vertexShader, fragmentShader);
  re->tex = generateGLTexture();

  re->model = load_obj(objFile);
  re->VAO = 0;

  glGenVertexArrays(1, &re->VAO);
  glBindVertexArray(re->VAO);

  re->positionVBO =
      generateVBO(&re->VAO, re->model->point_counts, 3, re->model->vp, 0);
  re->uvVBO =
      generateVBO(&re->VAO, re->model->point_counts, 2, re->model->vt, 1);

  return re;
}

void objModelFree(struct ObjModel *model) {
  glDeleteBuffers(1, &model->positionVBO);
  glDeleteBuffers(1, &model->uvVBO);

  glDeleteVertexArrays(1, &model->VAO);
  glDeleteProgram(model->shaderProgram);

  obj_free(model->model);
}

void objModelLoadTextureMap(struct ObjModel *model, const char *textureFile,
                            GLenum textureSlot, GLuint texLoc) {
  if (loadTexture(textureFile, textureSlot, model->tex) != 1) {
    printf("failed to load texture: %s\n", textureFile);
  }
}
