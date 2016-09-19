#ifndef SHADER_H
#define SHADER_H

#include <glib-2.0/glib.h>

//#include "obj_parser.h"

#include "../../general/src/general_helper.h"
#include "../../general/src/gl_helper.h"

struct Shader {
  GLuint shaderProgram;

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

struct Shader *shaderNew(const char *vertexShader, const char *fragmentShader);

void shaderFree(struct ObjModel *model);

// void objModelLoadTextureMap(struct ObjModel *model, char *textureFile,
//                            GLenum textureSlot, char *textureName);

gboolean shaderSetDiffuseTexture(struct Shader *shader, char *textureFile,
                                 shader GLenum textureSlot);

gboolean shaderSetSpecularTexture(struct Shader *shader, char *textureFile,
                                  shader GLenum textureSlot);

gboolean shaderSetNormalTexture(struct Shader *shader, char *textureFile,
                                GLenum textureSlot);

#endif
