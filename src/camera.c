#include "camera.h"
//#define ONE_DEG_IN_RAD (2.0f * M_PI) / 360.0f

#include <cmockery/pbc.h>

#ifdef UNIT_TESTING
#include <cmockery/cmockery_override.h>
#endif

static void calcCamT(struct Camera *camera) {
  // GLfloat *identityMat = identityMat4();
  // GLfloat *inversePosition =
  //    vec3New(-camera->position[0], -camera->position[1],
  //    -camera->position[2]);

  // camera->camT = translate(identityMat, inversePosition);

  // matFree(identityMat);
  // vecFree(inversePosition);
  REQUIRE(camera != NULL);

  mat4 identityMat = identityMat4();

  ENSURE(identityMat != NULL);

  vec3 inversePosition = vec3New(-vec3GetData(camera->position)[0],
                                 -vec3GetData(camera->position)[1],
                                 -vec3GetData(camera->position)[2]);

  ENSURE(inversePosition != NULL);

  camera->camT = translate(identityMat, inversePosition);

  ENSURE(camera->camT != NULL);

  matFree(identityMat);
  vecFree(inversePosition);
}

static void calcViewMat(struct Camera *camera) {
  REQUIRE(camera != NULL);

  // if (camera->viewMat != NULL) {
  matFree(camera->viewMat);
  //}

  camera->viewMat = mat4MulMat4(camera->camR, camera->camT);
  ENSURE(camera->viewMat != NULL);
}

static void calcProjMat(struct Camera *camera) {
  REQUIRE(camera != NULL);

  // if (camera->projMat != NULL) {
  matFree(camera->projMat);
  //}

  camera->projMat = perspective(
      camera->fov, (double)GL_WIDTH / (double)GL_HEIGHT, 0.1f, 100.0f);
  ENSURE(camera->projMat != NULL);
}

struct CameraMovement {
  double lon;
  double lat;
  double phi;
  double theta;
};

struct Camera *cameraNew(double speed, double fov) {
  // struct Camera *re = (struct Camera *)malloc(sizeof(struct Camera));
  struct Camera *re = (struct Camera *)DEFENSE_MALLOC(sizeof(struct Camera),
                                                      mallocFailAbort, NULL);
  // if (re == NULL) {
  //   return NULL;
  // }

  // void *movement = malloc(sizeof(struct CameraMovement));
  void *movement =
      DEFENSE_MALLOC(sizeof(struct CameraMovement), mallocFailAbort, NULL);
  // if (movement == NULL) {
  //   goto clean;
  // }

  ENSURE(re != NULL);
  ENSURE(re->movement != NULL);

  re->movement = movement;

  re->speed = speed;
  re->fov = fov;
  re->viewMat = NULL;
  re->projMat = NULL;

  re->position = vec3New(1.0f, 0.0f, 0.0f);
  ENSURE(re->position != NULL);
  // if (re->position == NULL) {
  //   goto clean;
  // }

  re->origin = vec3New(0.0f, 0.0f, 0.0f);
  ENSURE(re->origin != NULL);
  // if (re->origin == NULL) {
  //   goto clean;
  // }

  re->up = vec3New(0.0f, 1.0f, 0.0f);
  ENSURE(re->up != NULL);
  // if (re->up == NULL) {
  //   goto clean;
  // }

  calcCamT(re);
  // if (re->camT == NULL) {
  //   goto clean;
  // }
  ENSURE(re->camT != NULL);

  re->camR = lookAt(re->position, re->origin, re->up);
  // if (re->camR == NULL) {
  //   goto clean;
  // }

  ENSURE(re->camR != NULL);

  calcViewMat(re);
  // if (re->viewMat == NULL) {
  //   goto clean;
  // }

  ENSURE(re->viewMat != NULL);

  calcProjMat(re);
  // if (re->projMat == NULL) {
  //   goto clean;
  // }

  ENSURE(re->projMat != NULL);

  return re;

  // clean:
  //  cameraFree(re);
  //  return NULL;
}

void cameraFree(struct Camera *camera) {
  // REQUIRE(camera != NULL);

  vecFree(camera->position);
  vecFree(camera->movement);
  matFree(camera->camT);
  matFree(camera->camR);
  matFree(camera->viewMat);
  matFree(camera->projMat);

  free(camera);
}

mat4 cameraGetViewMat(struct Camera *camera) {
  REQUIRE(camera != NULL);

  calcViewMat(camera);
  return camera->viewMat;
}

mat4 cameraGetProjMat(struct Camera *camera) {
  REQUIRE(camera != NULL);

  calcProjMat(camera);
  return camera->projMat;
}

void cameraSetFOV(struct Camera *camera, double fov) {
  REQUIRE(camera != NULL);

  if (fov < CAMERA_FOV_MIN) {
    camera->fov = CAMERA_FOV_MIN;
  } else if (fov > CAMERA_FOV_MAX) {
    camera->fov = CAMERA_FOV_MAX;
  } else {
    camera->fov = fov;
  }
}

void cameraRotate(struct Camera *camera) {
  REQUIRE(camera != NULL);

  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;

  if (movement->lat > 85.0f) {
    movement->lat = 85.0f;
  } else if (movement->lat < -85.0f) {
    movement->lat = -85.0f;
  }

  movement->phi = (90.0f - movement->lat) * ONE_DEG_IN_RAD;
  movement->theta = movement->lon * ONE_DEG_IN_RAD;

  double positionX = 1.0f * sin(movement->phi) * cos(movement->theta);
  double positionY = 1.0f * cos(movement->phi);
  double positionZ = 1.0f * sin(movement->phi) * sin(movement->theta);

  vecFree(camera->position);
  camera->position = vec3New(positionX, positionY, positionZ);
  ENSURE(camera->position != NULL);

  calcCamT(camera);
  ENSURE(camera->camT != NULL);

  matFree(camera->camR);

  camera->camR = lookAt(camera->position, camera->origin, camera->up);
  ENSURE(camera->camR != NULL);

  calcViewMat(camera);
}

inline void cameraSetMovementLon(struct Camera *camera, double newLon) {
  REQUIRE(camera != NULL);

  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  movement->lon = newLon;
}

inline void cameraSetMovementLat(struct Camera *camera, double newLat) {
  REQUIRE(camera != NULL);

  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  movement->lat = newLat;
}

inline double cameraGetMovementLon(struct Camera *camera) {
  REQUIRE(camera != NULL);

  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  return movement->lon;
}

inline double cameraGetMovementLat(struct Camera *camera) {
  REQUIRE(camera != NULL);

  struct CameraMovement *movement = (struct CameraMovement *)camera->movement;
  return movement->lat;
}
