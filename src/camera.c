#include "camera.h"
#define ONE_DEG_IN_RAD (2.0f * M_PI) / 360.0f

static void calcCamT(struct Camera *camera) {
  GLfloat *identityMat = identityMat4();
  GLfloat *inversePosition =
      vec3New(-camera->position[0], -camera->position[1], -camera->position[2]);

  camera->camT = translate(identityMat, inversePosition);

  matFree(identityMat);
  vecFree(inversePosition);
}

static void calcViewMat(struct Camera *camera) {
  if (camera->viewMat != NULL) {
    matFree(camera->viewMat);
  }

  camera->viewMat = mat4MulMat4(camera->camR, camera->camT);
}

static void calcProjMat(struct Camera *camera) {
  if (camera->projMat != NULL) {
    matFree(camera->projMat);
  }

  camera->projMat = perspective(camera->fov, (float)GL_WIDTH / (float)GL_HEIGHT,
                                0.1f, 100.0f);
}

struct CameraMovement {
  double lon;
  double lat;
  double phi;
  double theta;
};

struct Camera *cameraNew(GLfloat speed, GLfloat fov) {
  struct Camera *re = (struct Camera *)defenseCalloc(1, sizeof(struct Camera));
  void *movement = defenseCalloc(1, sizeof(struct CameraMovement));

  re->movement = movement;

  re->speed = speed;
  re->fov = fov;
  re->viewMat = NULL;
  re->projMat = NULL;

  re->position = vec3New(1.0f, 0.0f, 0.0f);
  re->origin = vec3New(0.0f, 0.0f, 0.0f);
  re->up = vec3New(0.0f, 1.0f, 0.0f);

  calcCamT(re);

  re->camR = lookAt(re->position, re->origin, re->up);

  calcViewMat(re);
  calcProjMat(re);

  return re;
}

void cameraFree(struct Camera *camera) {
  vecFree(camera->position);
  matFree(camera->viewMat);
  matFree(camera->camT);
  matFree(camera->camR);
  matFree(camera->projMat);
}

GLfloat *cameraGetViewMat(struct Camera *camera) {
  calcViewMat(camera);
  return camera->viewMat;
}

GLfloat *cameraGetProjMat(struct Camera *camera) {
  calcProjMat(camera);
  return camera->projMat;
}

void cameraSetFOV(struct Camera *camera, GLfloat fov) {
  if (fov < CAMERA_FOV_MIN) {
    camera->fov = CAMERA_FOV_MIN;
  } else if (fov > CAMERA_FOV_MAX) {
    camera->fov = CAMERA_FOV_MAX;
  } else {
    camera->fov = fov;
  }
}

void cameraRotate(struct Camera *camera) {
  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  if (movement->lat > 85.0f) {
    movement->lat = 85.0f;
  } else if (movement->lat < -85.0f) {
    movement->lat = -85.0f;
  }

  movement->phi = (90.0f - movement->lat) * ONE_DEG_IN_RAD;
  movement->theta = movement->lon * ONE_DEG_IN_RAD;

  _FLOAT positionX = 1.0f * sin(movement->phi) * cos(movement->theta);
  _FLOAT positionY = 1.0f * cos(movement->phi);
  _FLOAT positionZ = 1.0f * sin(movement->phi) * sin(movement->theta);

  vecFree(camera->position);
  camera->position = vec3New(positionX, positionY, positionZ);
  calcCamT(camera);

  matFree(camera->camR);
  camera->camR = lookAt(camera->position, camera->origin, camera->up);

  calcViewMat(camera);
}

void cameraSetMovementLon(struct Camera *camera, double newLon) {
  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  movement->lon = newLon;
}

void cameraSetMovementLat(struct Camera *camera, double newLat) {
  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  movement->lat = newLat;
}

double cameraGetMovementLon(struct Camera *camera) {
  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  return movement->lon;
}

double cameraGetMovementLat(struct Camera *camera) {
  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  return movement->lat;
}
