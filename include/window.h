#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define MAIN_TYPE_WINDOW (main_window_get_type ())

G_DECLARE_FINAL_TYPE (MainWindow, main_window, MAIN, WINDOW,
                      AdwApplicationWindow)

GtkWindow *main_window_new (GtkApplication *app);

G_END_DECLS
