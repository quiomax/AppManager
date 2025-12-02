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

static void
on_quit_action (GSimpleAction *action G_GNUC_UNUSED,
                GVariant      *parameter G_GNUC_UNUSED,
                gpointer       user_data)
{
  GApplication *app = G_APPLICATION (user_data);
  g_application_quit (app);
}

static void
on_startup (GApplication *app)
{
  GSimpleAction *quit_action;
  const gchar *quit_accels[] = { "<Ctrl>Q", NULL };
  const gchar *refresh_accels[] = { "<Ctrl>R", NULL };
  const gchar *select_all_accels[] = { "<Ctrl>A", NULL };
  const gchar *select_all_global_accels[] = { "<Ctrl><Shift>A", NULL };
  const gchar *search_toggle_accels[] = { "<Ctrl>F", NULL };
  const gchar *remove_apps_accels[] = { "Delete", NULL };
  const gchar *cancel_accels[] = { "Escape", NULL };
  const gchar *focus_device_accels[] = { "<Ctrl>space", NULL };

  /* Create quit action */
  quit_action = g_simple_action_new ("quit", NULL);
  g_signal_connect (quit_action, "activate", G_CALLBACK (on_quit_action), app);
  g_action_map_add_action (G_ACTION_MAP (app), G_ACTION (quit_action));
  g_object_unref (quit_action);

  /* Set up keyboard accelerators */
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit", quit_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.refresh", refresh_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.select-all", select_all_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.select-all-global", select_all_global_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.search-toggle", search_toggle_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.remove-apps", remove_apps_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.cancel", cancel_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.focus-device", focus_device_accels);
}

int
main (int argc, char *argv[])
{
  g_autoptr(AdwApplication) app = NULL;

  app = adw_application_new ("com.muha.AppManager", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  g_signal_connect (app, "startup", G_CALLBACK (on_startup), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
