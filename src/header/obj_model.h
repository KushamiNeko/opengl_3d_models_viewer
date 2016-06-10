#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H

#include "obj_parser.h"
#include "gl_helper.h"

#include "../../../general/header/general_helper.h"

struct ObjModel {
  obj *model;
  GLuint shaderProgram;
  GLuint VAO;
  GLuint positionVBO;
  GLuint uvVBO;

  GLuint tex;
};

struct ObjModel *objModelNew(const char *objFile, const char *vertexShader,
                             const char *fragmentShader);

void objModelFree(struct ObjModel *model);

GLuint objModelGenTexLoc(GLenum textureSlot, const char *textureName);
void objModelLoadTextureMap(struct ObjModel *model, const char *textureFile,
                            GLenum textureSlot, GLuint texLoc);

#endif
