#ifndef SHADER_H
#define SHADER_H

//#include <glib-2.0/glib.h>

#include <stdbool.h>

#include "../../general/src/general_helper.h"
#include "../../general/src/gl_helper.h"

#include "../../general/src/debug_macro.h"

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

  const char **cubeMapFile;
  GLuint cubeMapTex;
  GLint cubeMapTexLoc;
};

struct Shader *shaderNew(const char *vertexShader, const char *fragmentShader);

void shaderFree(struct Shader *shader);

bool shaderSetDiffuseTexture(struct Shader *shader, char *textureFile,
                             GLenum textureSlot);

bool shaderSetSpecularTexture(struct Shader *shader, char *textureFile,
                              GLenum textureSlot);

bool shaderSetNormalTexture(struct Shader *shader, char *textureFile,
                            GLenum textureSlot);

bool shaderSetCubeMap(struct Shader *shader, const char *front,
                      const char *back, const char *top, const char *bottom,
                      const char *right, const char *left, GLenum textureSlot);

#endif
