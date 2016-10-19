#ifndef SHADER_H
#define SHADER_H

//#include <glib-2.0/glib.h>

#include <stdbool.h>

#include "../../general/src/general_helper.h"
#include "../../general/src/gl_helper.h"

#include "../../general/src/linear_math.h"

#include "../../general/src/debug_macro.h"

struct Shader {
  GLuint shaderProgram;

  GLint diffuseColorUniformLoc;
  int useDiffuseMap;
  vec3 diffuseColor;

  GLint useDiffuseMapLoc;

  char *diffTexFile;
  GLuint diffTex;
  GLint diffTexLoc;

  GLint specColorUniformLoc;
  int useSpecularMap;
  vec3 specularColor;

  GLint useSpecularMapLoc;

  char *specTexFile;
  GLuint specTex;
  GLint specTexLoc;

  //  GLint specGlossinessUniformLoc;
  //
  GLint useNormalMapLoc;
  int useNormalMap;

  char *normalTexFile;
  GLuint normalTex;
  GLint normalTexLoc;

  GLint normalIntensityUniformLoc;
  float normalIntensity;

  const char **cubeMapFile;
  GLuint cubeMapTex;
  GLint cubeMapTexLoc;
};

struct Shader *shaderNew(const char *vertexShader, const char *fragmentShader);

void shaderFree(struct Shader *shader);

void shaderSetUseDiffuseMap(struct Shader *shader, int active);

void shaderSetUseSpecularMap(struct Shader *shader, int active);

void shaderSetUseNormalMap(struct Shader *shader, int active);

void shaderSetDiffuseColor(struct Shader *shader, double r, double g, double b);

void shaderSetSpecularColor(struct Shader *shader, double r, double g,
                            double b);

void shaderSetNormalIntensity(struct Shader *shader, double intensity);

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
