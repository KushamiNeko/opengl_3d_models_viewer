#include "obj_parser.h"

#if __LIBRARY_PARSER
obj *load_obj(const char *file_name) {
  printf("Library parser\n");
  const struct aiScene *scene = aiImportFile(
      file_name, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
  if (!scene) {
    printf("ERROR on reading mesh: %s\n", file_name);
    return false;
  }

  const struct aiMesh *mesh = scene->mMeshes[0];
  printf("vertices in mesh: %i\n", mesh->mNumVertices);

  obj *model = (obj *)malloc(sizeof(obj));
  model->point_counts = mesh->mNumVertices;

  // float *vp = (float *)malloc(sizeof(float) * model->point_counts * 3);
  // float *vn = (float *)malloc(sizeof(float) * model->point_counts * 3);
  // float *vt = (float *)malloc(sizeof(float) * model->point_counts * 2);
  // float *vtan = (float *)malloc(sizeof(float) * model->point_counts * 4);

  float *vp = (float *)malloc(sizeof(float) * model->point_counts * 3);
  float *vn = (float *)malloc(sizeof(float) * model->point_counts * 3);
  float *vt = (float *)malloc(sizeof(float) * model->point_counts * 2);
  float *vtan = (float *)malloc(sizeof(float) * model->point_counts * 4);

  for (int i = 0; i < model->point_counts; i++) {
    const struct aiVector3D *vpos = &(mesh->mVertices[i]);
    vp[i * 3] = (GLfloat)vpos->x;
    vp[i * 3 + 1] = (GLfloat)vpos->y;
    vp[i * 3 + 2] = (GLfloat)vpos->z;

    const struct aiVector3D *vnor = &(mesh->mNormals[i]);
    vn[i * 3] = (GLfloat)vnor->x;
    vn[i * 3 + 1] = (GLfloat)vnor->y;
    vn[i * 3 + 2] = (GLfloat)vnor->z;

    // extract tangent and bitangent information from assimp
    const struct aiVector3D *tangent = &(mesh->mTangents[i]);
    const struct aiVector3D *bitangent = &(mesh->mBitangents[i]);

    vec3 t =
        vec3New((GLfloat)tangent->x, (GLfloat)tangent->y, (GLfloat)tangent->z);
    vec3 n = vec3New((GLfloat)vnor->x, (GLfloat)vnor->y, (GLfloat)vnor->z);
    vec3 b = vec3New((GLfloat)bitangent->x, (GLfloat)bitangent->y,
                     (GLfloat)bitangent->z);

    // orthogonalize and normalize the tangent
    // approximating a T, N, B inverse matrix
    vec3 t_i = vec3Normalize(vec3Sub(t, vec3MultFloat(n, vec3Dot(n, t))));
    // get determinant of T, B, N 3x3 matrix by dot * cross
    // this is a short-cut calculation
    // we can build a mat3 by using 3 vectors and get the determinant instead
    float det = (vec3Dot(vec3Cross(n, t), b));
    if (det < 0.0f) {
      det = -1.0f;
    } else {
      det = 1.0f;
    }

    vtan[i * 4] = vec3GetData(t_i)[0];
    vtan[i * 4 + 1] = vec3GetData(t_i)[1];
    vtan[i * 4 + 2] = vec3GetData(t_i)[2];
    vtan[i * 4 + 3] = det;

    vecFree(t);
    vecFree(n);
    vecFree(b);
    vecFree(t_i);

    const struct aiVector3D *vst = &(mesh->mTextureCoords[0][i]);
    vt[i * 2] = (GLfloat)vst->x;
    vt[i * 2 + 1] = (GLfloat)vst->y;
  }
  model->vp = vp;
  model->vn = vn;
  model->vt = vt;
  model->vtan = vtan;
  aiReleaseImport(scene);
  return model;
}

void obj_free(obj *model) {
  free(model->vp);
  free(model->vt);
  free(model->vn);
  free(model);
  model = NULL;
}

#else

void obj_free(obj *model) {
  free(model->vp);
  free(model->vt);
  free(model->vn);
  free(model);
}

typedef struct parsing {
  void *data;
  struct parsing *next;
} parsing;

typedef struct {
  parsing *content;
  parsing *last;
} parsing_head;

static parsing *parsing_new(void *data) {
  parsing *store = (parsing *)malloc(sizeof(parsing));
  store->data = data;
  store->next = NULL;
  return store;
}

static parsing_head *parsing_head_new(parsing *content) {
  parsing_head *store = (parsing_head *)malloc(sizeof(parsing_head));
  store->content = content;
  store->last = content;
  return store;
}

static void parsing_add(parsing_head *head, parsing *next_content) {
  if (head->content) {
    // printf("Content is FULL\n");
    head->last->next = next_content;
    head->last = next_content;
  } else {
    // printf("Content is NULL\n");
    head->content = next_content;
    head->last = next_content;
  }
}

static void parsing_free(parsing_head *head) {
  parsing *free_loc = head->content;
  while (1) {
    if (free_loc) {
      // printf("free something\n");
      parsing *temp = free_loc->next;
      free(free_loc->data);
      free(free_loc);
      free_loc = temp;
    } else {
      break;
    }
  }
  free(head);
}

static float **link_list_collect(int size, parsing_head *head) {
  float **temp = (float **)malloc(sizeof(float *) * size);

  parsing *store = head->content;
  int count = 0;
  while (1) {
    float *vertex = (float *)store->data;
    temp[count] = vertex;
    if (store->next) {
      store = store->next;
      count++;
    } else {
      break;
    }
  }
  return temp;
}

static void data_select_ouput(float *out_array, int data_size,
                              parsing_head *index_head,
                              float **data_collection) {
  parsing *store = index_head->content;
  int count = 0;
  while (1) {
    int *vertex_index = (int *)store->data;
    for (int i = 0; i < data_size; i++) {
      float *vertex = data_collection[vertex_index[i] - 1];
      for (int j = 0; j < data_size; j++) {
        out_array[count] = vertex[j];
        count++;
      }
    }
    if (store->next) {
      store = store->next;
    } else {
      break;
    }
  }
}

obj *load_obj(const char *file) {
  printf("My parser\n");
  FILE *f = fopen(file, "r");
  __ASSERT(f);

  obj *model = (obj *)malloc(sizeof(obj));
  parsing_head *vps = parsing_head_new(NULL);
  parsing_head *vno = parsing_head_new(NULL);
  parsing_head *vst = parsing_head_new(NULL);

  parsing_head *vpsin = parsing_head_new(NULL);
  parsing_head *vnoin = parsing_head_new(NULL);
  parsing_head *vstin = parsing_head_new(NULL);

  int p_count = 0;
  int st_count = 0;
  int n_count = 0;
  int f_count = 0;
  while (1) {
    char head[128];
    int res = fscanf(f, "%s", head);

    if (strcmp(head, "v") == 0) {
      float *vertex = (float *)malloc(sizeof(float) * 3);
      fscanf(f, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
      parsing *data = parsing_new(vertex);
      parsing_add(vps, data);
      // printf("Position: %f %f %f\n", vertex[0], vertex[1], vertex[2]);
      p_count++;
    } else if (strcmp(head, "vt") == 0) {
      float *vertex_st = (float *)malloc(sizeof(float) * 2);
      fscanf(f, "%f %f\n", &vertex_st[0], &vertex_st[1]);
      parsing *data = parsing_new(vertex_st);
      parsing_add(vst, data);
      // printf("ST: %f %f %f\n", vertex_st[0], vertex_st[1], vertex_st[2]);
      st_count++;
    } else if (strcmp(head, "vn") == 0) {
      float *vertex_nor = (float *)malloc(sizeof(float) * 3);
      fscanf(f, "%f %f %f\n", &vertex_nor[0], &vertex_nor[1], &vertex_nor[2]);
      parsing *data = parsing_new(vertex_nor);
      parsing_add(vno, data);
      // printf("Normal: %f %f %f\n", vertex_nor[0], vertex_nor[1],
      // vertex_nor[2]);
      n_count++;
    } else if (strcmp(head, "f") == 0) {
      int *vpin = (int *)malloc(sizeof(int) * 3);
      int *vuvin = (int *)malloc(sizeof(int) * 3);
      int *vnin = (int *)malloc(sizeof(int) * 3);

      if (p_count && st_count && n_count) {
        // printf("ALL Indices parse\n");
        fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vpin[0], &vuvin[0], &vnin[0],
               &vpin[1], &vuvin[1], &vnin[1], &vpin[2], &vuvin[2], &vnin[2]);
      } else if (p_count && n_count) {
        // printf("Position and Normal Indices parse\n");
        fscanf(f, "%d//%d %d//%d %d//%d\n", &vpin[0], &vnin[0], &vpin[1],
               &vnin[1], &vpin[2], &vnin[2]);
        vuvin[0] = vuvin[1] = vuvin[2] = 0;
      } else if (p_count) {
        // printf("Position Indices parse\n");
        fscanf(f, "%d// %d// %d//\n", &vpin[0], &vpin[1], &vpin[2]);
        vuvin[0] = vuvin[1] = vuvin[2] = 0;
        vnin[0] = vnin[1] = vnin[2] = 0;
      } else {
        // printf("No Indices parse\n");
        parsing_free(vst);
        parsing_free(vno);
        parsing_free(vps);

        parsing_free(vpsin);
        parsing_free(vnoin);
        parsing_free(vstin);

        obj_free(model);
        printf("There is nothing in this OBJ file!");
        return NULL;
      }
      if (res == EOF) {
        break;
      }

      f_count++;
      // printf("F_count: %i\n", f_count);
      parsing *data_vpin = parsing_new(vpin);
      parsing_add(vpsin, data_vpin);
      parsing *data_vuvin = parsing_new(vuvin);
      parsing_add(vstin, data_vuvin);
      parsing *data_vnin = parsing_new(vnin);
      parsing_add(vnoin, data_vnin);
      // printf("F: %d/%d/%d %d/%d/%d %d/%d/%d\n", vpin[0], vuvin[0], vnin[0],
      //        vpin[1], vuvin[1], vnin[1], vpin[2], vuvin[2], vnin[2]);
    }
  }
  fclose(f);

  model->point_counts = f_count * 3;
  printf("vertices in mesh: %i\n", model->point_counts);
  if (p_count) {
    float *vertex_position =
        (float *)malloc(sizeof(float) * 3 * model->point_counts);
    // model-<point_counts = number of triangles * number of points per triangle

    float **position_temp = link_list_collect(p_count, vps);
    data_select_ouput(vertex_position, 3, vpsin, position_temp);
    model->vp = vertex_position;
    free(position_temp);
  } else {
    model->vp = NULL;
  }
  parsing_free(vps);
  parsing_free(vpsin);

  if (n_count) {
    float *vertex_normal =
        (float *)malloc(sizeof(float) * 3 * model->point_counts);

    float **normal_temp = link_list_collect(n_count, vno);
    data_select_ouput(vertex_normal, 3, vnoin, normal_temp);
    model->vn = vertex_normal;
    free(normal_temp);
  } else {
    model->vn = NULL;
  }
  parsing_free(vno);
  parsing_free(vnoin);

  if (st_count) {
    float *vertex_st = (float *)malloc(sizeof(float) * 2 * model->point_counts);

    float **st_temp = link_list_collect(st_count, vst);
    data_select_ouput(vertex_st, 2, vstin, st_temp);
    model->vt = vertex_st;
    free(st_temp);
  } else {
    model->vt = NULL;
  }
  parsing_free(vst);
  parsing_free(vstin);

  return model;
}

#endif
