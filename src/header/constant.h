#ifndef CONSTANT_H
#define CONSTANT_H

#define OBJ_MODEL "sphere_base.obj"
#define TEXTURE "test_large.jpg"

#define VERTEX_SHADER "src/shader/vertex_shader.vert"
#define FRAGMENT_SHADER "src/shader/fragment_shader.frag"

#define CAMERA_SPEED 1.0f

enum {
  GL_WIDTH = 1440,
  GL_HEIGHT = 810,
  BOX_SPACE = 5,
  WIDGET_MARGIN = 15,
  GL_VERSION_MAJOR = 4,
  GL_VERSION_MINOR = 1
};

#endif
