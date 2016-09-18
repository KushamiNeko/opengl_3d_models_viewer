#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H

#include <glib-2.0/glib.h>

#include "obj_parser.h"

#include "../../general/src/general_helper.h"
#include "../../general/src/gl_helper.h"

struct ObjModel {
  obj *model;

  GLuint shaderProgram;

  GLuint VAO;
  GLuint positionVBO;
  GLuint uvVBO;
  GLuint normalVBO;
  GLuint tangentVBO;

  char *diffTexFile;
  GLuint diffTex;
  GLint diffTexLoc;

  char *specTexFile;
  GLuint specTex;
  GLint specTexLoc;

  char *normalTexFile;
  GLuint normalTex;
  GLint normalTexLoc;
};

struct ObjModel *objModelNew(const char *objFile, const char *vertexShader,
                             const char *fragmentShader);

void objModelFree(struct ObjModel *model);

// GLuint objModelGenTexLoc(GLenum textureSlot, const char *textureName);
// void objModelLoadTextureMap(struct ObjModel *model, const char *textureFile,
//                            GLenum textureSlot, GLuint texLoc);

// void objModelLoadTextureMap(struct ObjModel *model, char *textureFile,
//                            GLenum textureSlot, char *textureName);

gboolean objModelSetDiffuseTexture(struct ObjModel *model, char *textureFile,
                                   GLenum textureSlot);

gboolean objModelSetSpecularTexture(struct ObjModel *model, char *textureFile,
                                    GLenum textureSlot);

gboolean objModelSetNormalTexture(struct ObjModel *model, char *textureFile,
                                  GLenum textureSlot);

#endif
