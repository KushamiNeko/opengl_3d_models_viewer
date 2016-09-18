#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
//#include <GLFW/glfw3.h>

#include "../../general/src/general_helper.h"
#include "../../general/src/linear_math.h"

//#include "debug.h"
#include "constant.h"
//#include "linear_math.h"

struct Camera {
  vec3 position;
  vec3 origin;
  vec3 up;

  double fov;

  double speed;

  mat4 camT;
  mat4 camR;

  mat4 viewMat;
  mat4 projMat;

  void *movement;
};

struct Camera *cameraNew(double speed, double fov);

void cameraFree(struct Camera *camera);

void cameraRotate(struct Camera *camera);

void cameraSetFOV(struct Camera *camera, double fov);

mat4 cameraGetViewMat(struct Camera *camera);
mat4 cameraGetProjMat(struct Camera *camera);

inline void cameraSetMovementLon(struct Camera *camera, double newLon);
inline inline void cameraSetMovementLat(struct Camera *camera, double newLat);
inline inline double cameraGetMovementLon(struct Camera *camera);
inline inline double cameraGetMovementLat(struct Camera *camera);

#endif
