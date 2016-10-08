#include "obj_model.h"

//#include <cmockery/pbc.h>

#ifdef UNIT_TESTING
#include <cmockery/cmockery_override.h>
#endif

//#define DIFFUSE_TEXTURE "diff_tex"
//#define SPECULAR_TEXTURE "spec_tex"
//#define NORMAL_TEXTURE "normal_tex"

//#define DIFFUSE_TEXTURE_SLOT GL_TEXTURE0
//#define SPECULAR_TEXTURE_SLOT GL_TEXTURE1
//#define NORMAL_TEXTURE_SLOT GL_TEXTURE2

// static GLuint constShaderProgram(const char *vertFile, const char *fragFile)
// {
//  REQUIRE(vertFile != NULL);
//  REQUIRE(fragFile != NULL);
//
//  REQUIRE(g_file_test(vertFile, G_FILE_TEST_EXISTS));
//  REQUIRE(g_file_test(fragFile, G_FILE_TEST_EXISTS));
//
//  const char *vertexShader = readFile(vertFile);
//  const char *fragmentShader = readFile(fragFile);
//
//  REQUIRE(vertexShader != NULL);
//  REQUIRE(fragmentShader != NULL);
//
//  GLuint shaderProgram = glCreateProgram();
//  generateShader(&shaderProgram, vertexShader, GL_VERTEX_SHADER);
//  generateShader(&shaderProgram, fragmentShader, GL_FRAGMENT_SHADER);
//
//  LINK_GL_SHADER_PROGRAM(shaderProgram);
//
//  return shaderProgram;
//}

struct ObjModel *objModelNew(const char *objFile, struct Shader *shader) {
  REQUIRE(objFile != NULL);

  REQUIRE(shader != NULL);
  //  REQUIRE(vertexShader != NULL);
  //  REQUIRE(fragmentShader != NULL);

  // REQUIRE(g_file_test(objFile, G_FILE_TEST_EXISTS));
  REQUIRE(fileExist(objFile));
  //  REQUIRE(g_file_test(vertexShader, G_FILE_TEST_EXISTS));
  //  REQUIRE(g_file_test(fragmentShader, G_FILE_TEST_EXISTS));

  struct ObjModel *re =
      DEFENSE_MALLOC(sizeof(struct ObjModel), mallocFailAbort, NULL);

  ENSURE(re != NULL);

  //  re->diffTexFile = NULL;
  //  re->specTexFile = NULL;
  //  re->normalTexFile = NULL;

  // ENSURE(re->diffTexLoc != 0);
  // ENSURE(re->specTexLoc != 0);
  // ENSURE(re->normalTexLoc != 0);

  re->model = objLoad(objFile);

  ENSURE(re->model->vp != NULL);
  ENSURE(re->model->vn != NULL);
  ENSURE(re->model->vt != NULL);
  ENSURE(re->model->vtan != NULL);

  re->VAO = 0;

  glGenVertexArrays(1, &re->VAO);
  glBindVertexArray(re->VAO);

  re->positionVBO =
      generateVBO(&re->VAO, re->model->point_counts, 3, re->model->vp, 0);
  re->normalVBO =
      generateVBO(&re->VAO, re->model->point_counts, 3, re->model->vn, 1);
  re->uvVBO =
      generateVBO(&re->VAO, re->model->point_counts, 2, re->model->vt, 2);
  re->tangentVBO =
      generateVBO(&re->VAO, re->model->point_counts, 4, re->model->vtan, 3);

  re->shader = shader;

  //  re->shaderProgram = constShaderProgram(vertexShader, fragmentShader);
  //  ENSURE(re->shaderProgram != 0);
  //
  //  re->diffTex = generateGLTexture();
  //  re->specTex = generateGLTexture();
  //  re->normalTex = generateGLTexture();
  //
  //  ENSURE(re->diffTex != 0);
  //  ENSURE(re->specTex != 0);
  //  ENSURE(re->normalTex != 0);
  //
  //  re->diffTexLoc = generateTexLoc(&re->shaderProgram, DIFFUSE_TEXTURE);
  //  re->specTexLoc = generateTexLoc(&re->shaderProgram, SPECULAR_TEXTURE);
  //  re->normalTexLoc = generateTexLoc(&re->shaderProgram, NORMAL_TEXTURE);

  return re;
}

void objModelFree(struct ObjModel *model) {
  glDeleteBuffers(1, &model->positionVBO);
  glDeleteBuffers(1, &model->uvVBO);
  glDeleteBuffers(1, &model->normalVBO);

  //  glDeleteTextures(1, &model->diffTex);
  //  glDeleteTextures(1, &model->specTex);
  //  glDeleteTextures(1, &model->normalTex);

  glDeleteVertexArrays(1, &model->VAO);
  // glDeleteProgram(model->shaderProgram);

  objFree(model->model);
  DEFENSE_FREE(model);
}

// void objModelSetShader(struct ObjModel *model, struct Shader *shader) {
//  REQUIRE(model != NULL);
//  REQUIRE(shader != NULL);
//
//  model->shader = shader;
//
//}

// void objModelLoadTextureMap(struct ObjModel *model, char *textureFile,
//                            GLenum textureSlot, char *textureName) {
//  REQUIRE(textureFile != NULL);
//  REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
//
//  if (g_strcmp0(textureName, DIFFUSE_TEXTURE) == 0) {
//    REQUIRE(model->diffTex != 0);
//    REQUIRE(model->diffTexLoc != 0);
//
//    if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//                    &model->diffTex, &model->diffTexLoc) != 1) {
//      printf("failed to load texture: %s\n", textureFile);
//    } else {
//      model->diffTexFile = textureFile;
//    }
//
//  } else if (g_strcmp0(textureName, SPECULAR_TEXTURE) == 0) {
//    REQUIRE(model->specTex != 0);
//    REQUIRE(model->specTexLoc != 0);
//
//    if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//                    &model->specTex, &model->specTexLoc) != 1) {
//      printf("failed to load texture: %s\n", textureFile);
//    } else {
//      model->specTexFile = textureFile;
//    }
//
//  } else if (g_strcmp0(textureName, NORMAL_TEXTURE) == 0) {
//    REQUIRE(model->normalTex != 0);
//    REQUIRE(model->normalTexLoc != 0);
//
//    if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//                    &model->normalTex, &model->normalTexLoc) != 1) {
//      printf("failed to load texture: %s\n", textureFile);
//    } else {
//      model->normalTexFile = textureFile;
//    }
//
//  } else {
//    printf("IVALID TEXTURE NAME!\n");
//    printf("THIS SHOULD NEVER HAPPEN!\n");
//    exit(EXIT_FAILURE);
//  }
//
//  //  REQUIRE(model->tex != 0);
//  //  REQUIRE(model->texLoc != 0);
//  //
//  //  if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//  //  &model->tex,
//  //                  &model->texLoc) != 1) {
//  //    printf("failed to load texture: %s\n", textureFile);
//  //  } else {
//  //    model->textureFile = textureFile;
//  //  }
//}

// gboolean objModelSetDiffuseTexture(struct ObjModel *model, char *textureFile,
//                                   GLenum textureSlot) {
//  REQUIRE(textureFile != NULL);
//  REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
//
//  REQUIRE(model->diffTex != 0);
//  // REQUIRE(model->diffTexLoc != 0);
//
//  if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//                  &model->diffTex, &model->diffTexLoc) != 1) {
//    printf("failed to load texture: %s\n", textureFile);
//    return FALSE;
//  } else {
//    model->diffTexFile = textureFile;
//    return TRUE;
//  }
//}
//
// gboolean objModelSetSpecularTexture(struct ObjModel *model, char
// *textureFile,
//                                    GLenum textureSlot) {
//  REQUIRE(textureFile != NULL);
//  REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
//
//  REQUIRE(model->specTex != 0);
//  REQUIRE(model->specTexLoc != 0);
//
//  if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//                  &model->specTex, &model->specTexLoc) != 1) {
//    printf("failed to load texture: %s\n", textureFile);
//    return FALSE;
//  } else {
//    model->specTexFile = textureFile;
//    return TRUE;
//  }
//}
//
// gboolean objModelSetNormalTexture(struct ObjModel *model, char *textureFile,
//                                  GLenum textureSlot) {
//  REQUIRE(textureFile != NULL);
//  REQUIRE(g_file_test(textureFile, G_FILE_TEST_EXISTS));
//
//  REQUIRE(model->normalTex != 0);
//  REQUIRE(model->normalTexLoc != 0);
//
//  if (loadTexture(textureFile, &model->shaderProgram, textureSlot,
//                  &model->normalTex, &model->normalTexLoc) != 1) {
//    printf("failed to load texture: %s\n", textureFile);
//    return FALSE;
//  } else {
//    model->normalTexFile = textureFile;
//    return TRUE;
//  }
//}
