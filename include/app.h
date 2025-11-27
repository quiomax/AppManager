#pragma once

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define APP_TYPE_INFO (app_info_get_type ())
G_DECLARE_FINAL_TYPE (AppInfo, app_info, APP, INFO, GObject)

typedef enum {
  APP_TYPE_UNKNOWN,
  APP_TYPE_SYSTEM,
  APP_TYPE_USER
} AppType;

typedef enum {
  APP_CATEGORY_UNKNOWN,
  APP_CATEGORY_MALICIOUS,
  APP_CATEGORY_SAFE
} AppCategory;

struct _AppInfo {
  GObject parent_instance;
  gchar *package_name;
  gchar *label; /* Uygulama adı (daha sonra alınacak) */
  AppType type;
  AppCategory category;
  gboolean is_selected;
};

AppInfo *app_info_new (const gchar *package_name, AppType type);

const gchar *app_info_get_package_name (AppInfo *self);
const gchar *app_info_get_label (AppInfo *self);
AppType app_info_get_app_type (AppInfo *self);
AppCategory app_info_get_category (AppInfo *self);
void app_info_set_category (AppInfo *self, AppCategory category);

G_END_DECLS
