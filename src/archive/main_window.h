#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk-3.0/gtk/gtk.h>

#include <GL/glew.h>
//#include <GLFW/glfw3.h>

#include <stdlib.h>

#include "../../general/src/general_helper.h"

#include "camera.h"
#include "constant.h"
#include "obj_model.h"

struct MainWindow {
  GtkWidget *window;
  GtkWidget *container;
  GtkWidget *glArea;
  GtkWidget *button;

  struct ObjModel *model;
  struct Camera *camera;

  GLuint viewMatLoc;
  GLuint projMatLoc;

  GLuint texLoc;
};

struct MainWindow *mainWindowNew(const unsigned int glWidth,
                                 const unsigned int glHeight);
void mainWindowFree(struct MainWindow *window);

void mainWindowSetObjModel(struct MainWindow *mainWindow,
                           struct ObjModel *model);
void mainWindowSetCamera(struct MainWindow *mainWindow, struct Camera *camera);
void mainWindowGenTexLoc(struct MainWindow *mainWindow, GLuint *shaderProgram,
                         GLenum textureSlot, const char *textureName);

#endif
