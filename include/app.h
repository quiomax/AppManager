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
  gchar *label;
  AppType type;
  AppCategory category;
  gboolean is_selected;
  gchar *version;
  gchar *size;
  gchar *uid;
  gchar *install_date;
};

AppInfo *app_info_new (const gchar *package_name, AppType type);

const gchar *app_info_get_package_name (AppInfo *self);
const gchar *app_info_get_label (AppInfo *self);
AppType app_info_get_app_type (AppInfo *self);
AppCategory app_info_get_category (AppInfo *self);
void app_info_set_category (AppInfo *self, AppCategory category);

const gchar *app_info_get_version (AppInfo *self);
void app_info_set_version (AppInfo *self, const gchar *version);

const gchar *app_info_get_size (AppInfo *self);
void app_info_set_size (AppInfo *self, const gchar *size);

const gchar *app_info_get_uid (AppInfo *self);
void app_info_set_uid (AppInfo *self, const gchar *uid);

const gchar *app_info_get_install_date (AppInfo *self);
void app_info_set_install_date (AppInfo *self, const gchar *install_date);

G_END_DECLS
