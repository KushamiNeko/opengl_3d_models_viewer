#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
//#include <GLFW/glfw3.h>

#include "../../../general/header/general_helper.h"

#include "debug.h"
#include "constant.h"
#include "linear_math.h"

struct Camera {
  GLfloat *position;
  GLfloat *origin;
  GLfloat *up;

  GLfloat fov;

  GLfloat speed;

  GLfloat *camT;
  GLfloat *camR;

  GLfloat *viewMat;
  GLfloat *projMat;

  void *movement;
};

struct Camera *cameraNew(GLfloat speed, GLfloat fov);
void cameraFree(struct Camera *camera);

// void cameraRotateX(struct Camera *camera, GLfloat degree);
void cameraRotate(struct Camera *camera);
// void cameraRotateZ(struct Camera *camera, GLfloat degree);

void cameraSetFOV(struct Camera *camera, GLfloat fov);

GLfloat *cameraGetViewMat(struct Camera *camera);
GLfloat *cameraGetProjMat(struct Camera *camera);

void cameraSetMovementLon(struct Camera *camera, double newLon);
void cameraSetMovementLat(struct Camera *camera, double newLat);

double cameraGetMovementLon(struct Camera *camera);
double cameraGetMovementLat(struct Camera *camera);

#endif
