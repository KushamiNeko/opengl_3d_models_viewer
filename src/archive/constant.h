#ifndef CONSTANT_H
#define CONSTANT_H

#define OBJ_MODEL "resource/sphere_base.obj"
#define TEXTURE "resource/frozen_waterfall.jpg"

#define VERTEX_SHADER "src/shader/vertex_shader.vert"
#define FRAGMENT_SHADER "src/shader/fragment_shader.frag"

#define CAMERA_ROTATE_SPEED 0.1f
#define CAMERA_SCROLL_SPEED 3.0f

#define CAMERA_FOV_MIN 20.0f
#define CAMERA_FOV_MAX 100.0f

enum {
  GL_WIDTH = 1600,
  GL_HEIGHT = 900,
  BOX_SPACE = 5,
  WIDGET_MARGIN = 15,
  GL_VERSION_MAJOR = 4,
  GL_VERSION_MINOR = 1
};

#endif
