#include <adwaita.h>
#include "window.h"

static void
on_activate (GApplication *app)
{
  GtkWindow *window;

  window = gtk_application_get_active_window (GTK_APPLICATION (app));
  if (window) {
    gtk_window_present (window);
    return;
  }

  window = main_window_new (GTK_APPLICATION (app));
  gtk_window_present (window);
}

int
main (int argc, char *argv[])
{
  g_autoptr(AdwApplication) app = NULL;

  app = adw_application_new ("com.muha.AppManager", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
