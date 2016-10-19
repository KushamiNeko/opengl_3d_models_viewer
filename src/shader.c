#include "shader.h"

//#include <cmockery/pbc.h>

#ifdef UNIT_TESTING
#include <cmockery/cmockery_override.h>
#endif

#define DIFFUSE_TEXTURE "diff_tex"
#define SPECULAR_TEXTURE "spec_tex"
#define NORMAL_TEXTURE "normal_tex"

#define USE_DIFFUSE_MAP "use_diffuse_map"
#define USE_SPECULAR_MAP "use_specular_map"
#define USE_NORMAL_MAP "use_normal_map"

#define DIFFUSE_COLOR "diff_color"
#define SPECULAR_COLOR "spec_color"

#define NORMAL_INTENSITY "normal_intensity"

#define CUBE_MAP_TEXTURE "cube_map_tex"

static GLuint constShaderProgram(const char *vertFile, const char *fragFile) {
  REQUIRE(vertFile != NULL);
  REQUIRE(fragFile != NULL);

  // REQUIRE(g_file_test(vertFile, G_FILE_TEST_EXISTS));
  // REQUIRE(g_file_test(fragFile, G_FILE_TEST_EXISTS));

  REQUIRE(fileExist(vertFile));
  REQUIRE(fileExist(fragFile));

  const char *vertexShader = fileReadContents(vertFile);
  const char *fragmentShader = fileReadContents(fragFile);

  REQUIRE(vertexShader != NULL);
  REQUIRE(fragmentShader != NULL);

  GLuint shaderProgram = glCreateProgram();
  generateShader(&shaderProgram, vertexShader, GL_VERTEX_SHADER);
  generateShader(&shaderProgram, fragmentShader, GL_FRAGMENT_SHADER);

  LINK_GL_SHADER_PROGRAM(shaderProgram);

  return shaderProgram;
}

struct Shader *shaderNew(const char *vertexShader, const char *fragmentShader) {
  REQUIRE(vertexShader != NULL);
  REQUIRE(fragmentShader != NULL);

  //  REQUIRE(g_file_test(vertexShader, G_FILE_TEST_EXISTS));
  //  REQUIRE(g_file_test(fragmentShader, G_FILE_TEST_EXISTS));

  REQUIRE(fileExist(vertexShader));
  REQUIRE(fileExist(fragmentShader));

  struct Shader *re =
      DEFENSE_MALLOC(sizeof(struct Shader), mallocFailAbort, NULL);

  ENSURE(re != NULL);

  re->diffTexFile = NULL;
  re->specTexFile = NULL;
  re->normalTexFile = NULL;
  re->cubeMapFile = DEFENSE_MALLOC(6 * sizeof(char *), mallocFailAbort, NULL);
  ENSURE(re->cubeMapFile != NULL);

  re->shaderProgram = constShaderProgram(vertexShader, fragmentShader);
  ENSURE(re->shaderProgram != 0);

  re->diffTex = generateGLTexture();
  re->specTex = generateGLTexture();
  re->normalTex = generateGLTexture();
  re->cubeMapTex = generateGLTexture();

  ENSURE(re->diffTex != 0);
  ENSURE(re->specTex != 0);
  ENSURE(re->normalTex != 0);
  ENSURE(re->cubeMapTex != 0);

  re->diffTexLoc = generateTexLoc(&re->shaderProgram, DIFFUSE_TEXTURE);
  re->specTexLoc = generateTexLoc(&re->shaderProgram, SPECULAR_TEXTURE);
  re->normalTexLoc = generateTexLoc(&re->shaderProgram, NORMAL_TEXTURE);
  re->cubeMapTexLoc = generateTexLoc(&re->shaderProgram, CUBE_MAP_TEXTURE);

  glUseProgram(re->shaderProgram);

  re->useDiffuseMapLoc =
      glGetUniformLocation(re->shaderProgram, USE_DIFFUSE_MAP);

  re->useSpecularMapLoc =
      glGetUniformLocation(re->shaderProgram, USE_SPECULAR_MAP);

  re->useNormalMapLoc = glGetUniformLocation(re->shaderProgram, USE_NORMAL_MAP);

  re->useDiffuseMap = 0;
  re->useSpecularMap = 0;
  re->useNormalMap = 0;

  glUniform1f(re->useDiffuseMapLoc, re->useDiffuseMap);
  glUniform1f(re->useSpecularMapLoc, re->useSpecularMap);
  glUniform1f(re->useNormalMapLoc, re->useNormalMap);

  re->diffuseColorUniformLoc =
      glGetUniformLocation(re->shaderProgram, DIFFUSE_COLOR);

  re->specColorUniformLoc =
      glGetUniformLocation(re->shaderProgram, SPECULAR_COLOR);

  //  re->specGlossinessUniformLoc =
  //      glGetUniformLocation(re->shaderProgram, "specGloss");

  re->normalIntensityUniformLoc =
      glGetUniformLocation(re->shaderProgram, NORMAL_INTENSITY);

  // re->diffuseColor = vec3New(1.0f, 1.0f, 1.0f);
  re->diffuseColor = vec3New(0.0f, 0.0f, 0.0f);
  re->specularColor = vec3New(1.0f, 1.0f, 1.0f);

  re->normalIntensity = 0.0f;

  glUseProgram(re->shaderProgram);
  glUniform3fv(re->diffuseColorUniformLoc, 1, vec3GetData(re->diffuseColor));
  glUniform3fv(re->specColorUniformLoc, 1, vec3GetData(re->specularColor));

  glUniform1f(re->normalIntensityUniformLoc, re->normalIntensity);

  return re;
}

void shaderFree(struct Shader *shader) {
  glDeleteTextures(1, &shader->diffTex);
  glDeleteTextures(1, &shader->specTex);
  glDeleteTextures(1, &shader->normalTex);

  glDeleteTextures(1, &shader->cubeMapTex);

  free(shader->cubeMapFile);

  glDeleteProgram(shader->shaderProgram);

  DEFENSE_FREE(shader);
}

void shaderSetUseDiffuseMap(struct Shader *shader, int active) {
  glUseProgram(shader->shaderProgram);

  shader->useDiffuseMap = active;
  glUniform1i(shader->useDiffuseMapLoc, shader->useDiffuseMap);
}

void shaderSetUseSpecularMap(struct Shader *shader, int active) {
  glUseProgram(shader->shaderProgram);

  // glDeleteTextures(1, &shader->specTex);

  shader->useSpecularMap = active;
  glUniform1i(shader->useSpecularMapLoc, shader->useSpecularMap);
}

void shaderSetUseNormalMap(struct Shader *shader, int active) {
  glUseProgram(shader->shaderProgram);

  // glDeleteTextures(1, &shader->normalTex);

  shader->useNormalMap = active;
  glUniform1i(shader->useNormalMapLoc, shader->useNormalMap);
}

void shaderSetDiffuseColor(struct Shader *shader, double r, double g,
                           double b) {
  vec3GetData(shader->diffuseColor)[0] = r;
  vec3GetData(shader->diffuseColor)[1] = g;
  vec3GetData(shader->diffuseColor)[2] = b;

  glUseProgram(shader->shaderProgram);
  glUniform3fv(shader->diffuseColorUniformLoc, 1,
               vec3GetData(shader->diffuseColor));
}

void shaderSetSpecularColor(struct Shader *shader, double r, double g,
                            double b) {
  vec3GetData(shader->specularColor)[0] = r;
  vec3GetData(shader->specularColor)[1] = g;
  vec3GetData(shader->specularColor)[2] = b;

  glUseProgram(shader->shaderProgram);
  glUniform3fv(shader->specColorUniformLoc, 1,
               vec3GetData(shader->specularColor));
}

void shaderSetNormalIntensity(struct Shader *shader, double intensity) {
  glUseProgram(shader->shaderProgram);
  shader->normalIntensity = intensity;
  glUniform1f(shader->normalIntensityUniformLoc, shader->normalIntensity);
}

// static bool freeDiffTex = false;
// static bool freeSpecTex = false;
// static bool freeNorTex = false;

bool shaderSetDiffuseTexture(struct Shader *shader, char *textureFile,
                             GLenum textureSlot) {
  REQUIRE(textureFile != NULL);
  // REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
  REQUIRE(fileExist(textureFile));

  REQUIRE(shader->diffTex != 0);

  //  if (!freeDiffTex) {
  //    freeDiffTex = true;
  //  } else {
  //    glDeleteTextures(1, &shader->diffTex);
  //  }

  if (loadTexture(textureFile, &shader->shaderProgram, textureSlot,
                  &shader->diffTex, &shader->diffTexLoc) != 1) {
    printf("failed to load texture: %s\n", textureFile);
    return false;
  } else {
    shader->diffTexFile = textureFile;
    return true;
  }
}

bool shaderSetSpecularTexture(struct Shader *shader, char *textureFile,
                              GLenum textureSlot) {
  REQUIRE(textureFile != NULL);
  // REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
  REQUIRE(fileExist(textureFile));

  REQUIRE(shader->specTex != 0);
  // REQUIRE(shader->specTexLoc != 0);

  //  if (!freeSpecTex) {
  //    freeSpecTex = true;
  //  } else {
  //    glDeleteTextures(1, &shader->specTex);
  //  }

  if (loadTexture(textureFile, &shader->shaderProgram, textureSlot,
                  &shader->specTex, &shader->specTexLoc) != 1) {
    printf("failed to load texture: %s\n", textureFile);
    return false;
  } else {
    shader->specTexFile = textureFile;
    return true;
  }
}

bool shaderSetNormalTexture(struct Shader *shader, char *textureFile,
                            GLenum textureSlot) {
  REQUIRE(textureFile != NULL);
  // REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
  REQUIRE(fileExist(textureFile));

  REQUIRE(shader->normalTex != 0);
  // REQUIRE(shader->normalTexLoc != 0);

  //  if (!freeNorTex) {
  //    freeNorTex = true;
  //  } else {
  //    glDeleteTextures(1, &shader->normalTex);
  //  }

  if (loadTexture(textureFile, &shader->shaderProgram, textureSlot,
                  &shader->normalTex, &shader->normalTexLoc) != 1) {
    printf("failed to load texture: %s\n", textureFile);
    return false;
  } else {
    shader->normalTexFile = textureFile;
    return true;
  }
}

bool shaderSetCubeMap(struct Shader *shader, const char *front,
                      const char *back, const char *top, const char *bottom,
                      const char *right, const char *left, GLenum textureSlot) {
  REQUIRE(front != NULL);
  REQUIRE(back != NULL);
  REQUIRE(top != NULL);
  REQUIRE(bottom != NULL);
  REQUIRE(right != NULL);
  REQUIRE(left != NULL);

  REQUIRE(fileExist(front));
  REQUIRE(fileExist(back));
  REQUIRE(fileExist(top));
  REQUIRE(fileExist(bottom));
  REQUIRE(fileExist(right));
  REQUIRE(fileExist(left));

  // REQUIRE(g_file_test(front, G_FILE_TEST_EXISTS));
  // REQUIRE(g_file_test(back, G_FILE_TEST_EXISTS));
  // REQUIRE(g_file_test(top, G_FILE_TEST_EXISTS));
  // REQUIRE(g_file_test(bottom, G_FILE_TEST_EXISTS));
  // REQUIRE(g_file_test(right, G_FILE_TEST_EXISTS));
  // REQUIRE(g_file_test(left, G_FILE_TEST_EXISTS));

  REQUIRE(shader->cubeMapTex != 0);

  if (createCubeMap(front, back, top, bottom, right, left,
                    &shader->shaderProgram, textureSlot, &shader->cubeMapTex,
                    &shader->cubeMapTexLoc) != 1) {
    printf("failed to load cube map\n");
    return false;
  } else {
    shader->cubeMapFile[0] = front;
    shader->cubeMapFile[1] = back;
    shader->cubeMapFile[2] = top;
    shader->cubeMapFile[3] = bottom;
    shader->cubeMapFile[4] = right;
    shader->cubeMapFile[5] = left;

    return true;
  }
}
