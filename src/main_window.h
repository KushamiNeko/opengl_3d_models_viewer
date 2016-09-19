#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glib-2.0/glib.h>


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../general/src/linear_math.h"
#include "obj_model.h"


struct MainWindow *mainWindowInit(int argc, char const *argv[]);

void mainWindowDrawingLoop(struct MainWindow *mainWindow, struct ObjModel **model, struct Camera *camera);
