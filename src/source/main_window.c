#include "../header/main_window.h"

static gboolean MOUSE_CLICKED = FALSE;
static double MOUSE_START_X = 0.0f;
static double MOUSE_START_Y = 0.0f;

static double MOUSE_START_LON = 0.0f;
static double MOUSE_START_LAT = 0.0f;

static gboolean mouseClicked(GtkWidget *widget, GdkEvent *event,
                             void *userData) {
  struct MainWindow *user = (struct MainWindow *)userData;
  MOUSE_CLICKED = TRUE;
  MOUSE_START_X = event->motion.x;
  MOUSE_START_Y = event->motion.y;
  MOUSE_START_LON = cameraGetMovementLon(user->camera);
  MOUSE_START_LAT = cameraGetMovementLat(user->camera);
}

static gboolean motionNotify(GtkWidget *widget, GdkEvent *event,
                             void *userData) {
  if (MOUSE_CLICKED) {
    struct MainWindow *user = (struct MainWindow *)userData;

    double motionX = event->motion.x - MOUSE_START_X;
    double motionY = event->motion.y - MOUSE_START_Y;

    cameraSetMovementLon(user->camera, (motionX * 0.1) + MOUSE_START_LON);
    cameraSetMovementLat(user->camera, (motionY * 0.1) + MOUSE_START_LAT);

    cameraRotate(user->camera);

    gtk_gl_area_queue_render(GTK_GL_AREA(user->glArea));
  }
}

static gboolean mouseReleased(GtkWidget *widget, GdkEvent *event,
                              void *userData) {
  MOUSE_CLICKED = FALSE;
}

static void updateViewMat(struct MainWindow *mainWindow) {
  glUniformMatrix4fv(mainWindow->viewMatLoc, 1, GL_FALSE,
                     cameraGetViewMat(mainWindow->camera));
}

static void updateProjMat(struct MainWindow *mainWindow) {
  glUniformMatrix4fv(mainWindow->projMatLoc, 1, GL_FALSE,
                     cameraGetProjMat(mainWindow->camera));
}

static void sliderRangeChanged(GtkRange *range, void *userData) {
  struct MainWindow *user = (struct MainWindow *)userData;
  double value = gtk_range_get_value(range);
  cameraSetFOV(user->camera, (GLfloat)value);
  gtk_gl_area_queue_render(GTK_GL_AREA(user->glArea));
}

static gboolean glRender(GtkGLArea *area, GdkGLContext *context,
                         void *userData) {
  struct MainWindow *user = (struct MainWindow *)userData;

  gdk_gl_context_make_current(context);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  // update the gl data within the main render loop to prevent gtk to freeze

  glUseProgram(user->model->shaderProgram);

  updateViewMat(user);
  updateProjMat(user);

  bindToTexLoc(user->texLoc, GL_TEXTURE0, &user->model->shaderProgram);

  glBindVertexArray(user->model->VAO);
  glDrawArrays(GL_TRIANGLES, 0, user->model->model->point_counts);

  // drawing wireframe
  // glDrawArrays(GL_LINES, 0, user->pattern->vertexCounts);

  glFlush();
  return TRUE;
}

static gchar *getTextureFile(GtkWindow *parentWindow) {
  GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Open File", parentWindow, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel",
      GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

  gchar *fileName = NULL;
  gint response = gtk_dialog_run(GTK_DIALOG(dialog));
  if (response == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    fileName = gtk_file_chooser_get_filename(chooser);
  }

  gtk_widget_destroy(dialog);
  return fileName;
}

static void buttonClicked(GtkButton *button, void *userData) {
  struct MainWindow *user = (struct MainWindow *)userData;
  gchar *textureFile = getTextureFile(GTK_WINDOW(user->window));

  if (textureFile == NULL) {
    return;
  }

  g_print("%s\n", textureFile);

  GLuint *tex;
  *tex = user->model->tex;
  glDeleteTextures(1, tex);

  objModelLoadTextureMap(user->model, textureFile, GL_TEXTURE0, user->texLoc);
  gtk_gl_area_queue_render(GTK_GL_AREA(user->glArea));
}

static void glRealize(GtkGLArea *area) {
  g_print("initialize GL area\n");
  gtk_gl_area_make_current(area);

  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  g_print("OpenGL Renderer: %s\n", renderer);
  g_print("OpenGL Version: %s\n", version);

  glClearColor(0, 0, 0, 0);

  if (gtk_gl_area_get_error(area) != NULL) {
    g_print("GL Area initialization failed!\n");
    return;
  }

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  // depth testing interprets a smaller value as "closer"
  glDepthFunc(GL_LESS);

  // enable back face culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
}

struct MainWindow *mainWindowNew(const unsigned int glWidth,
                                 const unsigned int glHeight) {
  struct MainWindow *re =
      (struct MainWindow *)defenseCalloc(1, sizeof(struct MainWindow));

  re->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(re->window), "GTK HDRI Viewer");
  gtk_window_set_position(GTK_WINDOW(re->window), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(re->window), FALSE);

  gtk_widget_set_events(
      re->window, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY | GDK_BUTTON_PRESS_MASK |
                      GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

  re->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, BOX_SPACE);
  gtk_widget_set_margin_bottom(GTK_WIDGET(re->container), WIDGET_MARGIN);
  gtk_widget_set_margin_top(GTK_WIDGET(re->container), WIDGET_MARGIN);
  gtk_widget_set_margin_start(GTK_WIDGET(re->container), WIDGET_MARGIN);
  gtk_widget_set_margin_end(GTK_WIDGET(re->container), WIDGET_MARGIN);
  gtk_container_add(GTK_CONTAINER(re->window), re->container);

  re->glArea = gtk_gl_area_new();
  gtk_container_add(GTK_CONTAINER(re->container), re->glArea);
  gtk_gl_area_set_required_version(GTK_GL_AREA(re->glArea), GL_VERSION_MAJOR,
                                   GL_VERSION_MINOR);
  gtk_widget_set_size_request(re->glArea, glWidth, glHeight);

  // instead of entering GL render loop, we queue the render command when we
  // actually need it
  gtk_gl_area_set_auto_render(GTK_GL_AREA(re->glArea), FALSE);

  // function for initializing Gtk GL area

  re->slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20.0f,
                                        90.0f, 0.000001f);
  gtk_container_add(GTK_CONTAINER(re->container), re->slider);

  re->button = gtk_button_new_with_label("Choose Texture");
  gtk_container_add(GTK_CONTAINER(re->container), re->button);

  g_signal_connect(GTK_WIDGET(re->window), "destroy", G_CALLBACK(gtk_main_quit),
                   NULL);
  g_signal_connect(re->window, "button-press-event", G_CALLBACK(mouseClicked),
                   re);
  g_signal_connect(re->window, "motion-notify-event", G_CALLBACK(motionNotify),
                   re);
  g_signal_connect(re->window, "button-release-event",
                   G_CALLBACK(mouseReleased), re);

  g_signal_connect(re->glArea, "realize", G_CALLBACK(glRealize), NULL);
  g_signal_connect(re->glArea, "render", G_CALLBACK(glRender), re);
  g_signal_connect(re->slider, "value-changed", G_CALLBACK(sliderRangeChanged),
                   re);
  g_signal_connect(re->button, "clicked", G_CALLBACK(buttonClicked), re);

  gtk_widget_show_all(re->window);

  return re;
}

void mainWindowFree(struct MainWindow *mainWindow) {
  gtk_widget_destroy(mainWindow->window);
}

void mainWindowSetObjModel(struct MainWindow *mainWindow,
                           struct ObjModel *model) {
  mainWindow->model = model;

  mainWindow->viewMatLoc =
      glGetUniformLocation(model->shaderProgram, "view_mat");
  mainWindow->projMatLoc =
      glGetUniformLocation(model->shaderProgram, "proj_mat");

  mainWindowGenTexLoc(mainWindow, &model->shaderProgram, GL_TEXTURE0,
                      "diff_tex");

  gtk_gl_area_queue_render(GTK_GL_AREA(mainWindow->glArea));
}

void mainWindowSetCamera(struct MainWindow *mainWindow, struct Camera *camera) {
  mainWindow->camera = camera;
  gtk_range_set_value(GTK_RANGE(mainWindow->slider), camera->fov);

  updateViewMat(mainWindow);
  updateProjMat(mainWindow);

  gtk_gl_area_queue_render(GTK_GL_AREA(mainWindow->glArea));
}

void mainWindowGenTexLoc(struct MainWindow *mainWindow, GLuint *shaderProgram,
                         GLenum textureSlot, const char *textureName) {
  mainWindow->texLoc = generateTexLoc(textureSlot, shaderProgram, textureName);
}
