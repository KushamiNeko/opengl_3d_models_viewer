#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <GL/glew.h>
#//include <GLFW/glfw3.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "linear_math.h"
#include "../../general/src/general_helper.h"
#include "../../general/src/linear_math.h"

/* #ifndef __DEBUG */
/* #include "debug.h" */
/* #endif */

#define __LIBRARY_PARSER 1

typedef struct {
  int point_counts;
  float *vp;
  float *vn;
  float *vt;
  float *vtan;
} obj;

void obj_free(obj *model);
obj *load_obj(const char *file);

#if __LIBRARY_PARSER
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#endif

#endif
