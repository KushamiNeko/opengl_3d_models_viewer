#include "camera.h"
#include "constant.h"
#include "main_window.h"
#include "obj_model.h"

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);

  struct MainWindow *window = mainWindowNew(GL_WIDTH, GL_HEIGHT);

  struct Camera *camera = cameraNew(CAMERA_ROTATE_SPEED, 77.0f);
  struct ObjModel *model =
      objModelNew(OBJ_MODEL, VERTEX_SHADER, FRAGMENT_SHADER);

  mainWindowSetObjModel(window, model);
  mainWindowSetCamera(window, camera);

  objModelLoadTextureMap(model, TEXTURE, GL_TEXTURE0, window->texLoc);

  gtk_main();

  return 0;
}
