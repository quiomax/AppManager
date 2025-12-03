#include "app.h"

G_DEFINE_FINAL_TYPE (AppInfo, app_info, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_PACKAGE_NAME,
  PROP_APP_TYPE,
  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS] = { NULL, };

static void
app_info_finalize (GObject *object)
{
  AppInfo *self = APP_INFO (object);
  g_free (self->package_name);
  g_free (self->label);
  g_free (self->version);
  g_free (self->size);
  g_free (self->uid);
  g_free (self->install_date);
  G_OBJECT_CLASS (app_info_parent_class)->finalize (object);
}

static void
app_info_get_property (GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  AppInfo *self = APP_INFO (object);

  switch (property_id)
    {
    case PROP_PACKAGE_NAME:
      g_value_set_string (value, self->package_name);
      break;
    case PROP_APP_TYPE:
      g_value_set_int (value, self->type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
app_info_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  AppInfo *self = APP_INFO (object);

  switch (property_id)
    {
    case PROP_PACKAGE_NAME:
      self->package_name = g_value_dup_string (value);
      break;
    case PROP_APP_TYPE:
      self->type = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
app_info_class_init (AppInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = app_info_finalize;
  object_class->get_property = app_info_get_property;
  object_class->set_property = app_info_set_property;

  obj_properties[PROP_PACKAGE_NAME] =
    g_param_spec_string ("package-name",
                         "Package Name",
                         "The package name of the application",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  obj_properties[PROP_APP_TYPE] =
    g_param_spec_int ("app-type",
                      "App Type",
                      "The type of the application",
                      APP_TYPE_UNKNOWN,
                      APP_TYPE_USER,
                      APP_TYPE_UNKNOWN,
                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

#include "utils.h"

static void
app_info_init (AppInfo *self)
{
  self->type = APP_TYPE_UNKNOWN;
  self->category = APP_CATEGORY_UNKNOWN;
  self->is_selected = FALSE;
  self->is_enabled = TRUE;
}

AppInfo *
app_info_new (const gchar *package_name,
              AppType      type)
{
  AppInfo *self = g_object_new (APP_TYPE_INFO, NULL);

  self->package_name = g_strdup (package_name);
  self->type = type;

  /* Label'ı utils'den al (Dosyadan veya otomatik formatla) */
  self->label = utils_get_app_name (package_name);

  /* Kategoriyi utils'den al */
  self->category = utils_get_app_category (package_name);

  return self;
}

const gchar *
app_info_get_package_name (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), NULL);
  return self->package_name;
}

const gchar *
app_info_get_label (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), NULL);
  return self->label;
}

AppType
app_info_get_app_type (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), APP_TYPE_UNKNOWN);
  return self->type;
}

AppCategory
app_info_get_category (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), APP_CATEGORY_UNKNOWN);
  return self->category;
}

void
app_info_set_category (AppInfo    *self,
                       AppCategory category)
{
  g_return_if_fail (APP_IS_INFO (self));
  self->category = category;
}

const gchar *
app_info_get_version (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), NULL);
  return self->version;
}

void
app_info_set_version (AppInfo     *self,
                      const gchar *version)
{
  g_return_if_fail (APP_IS_INFO (self));
  g_free (self->version);
  self->version = g_strdup (version);
}

const gchar *
app_info_get_size (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), NULL);
  return self->size;
}

void
app_info_set_size (AppInfo     *self,
                   const gchar *size)
{
  g_return_if_fail (APP_IS_INFO (self));
  g_free (self->size);
  self->size = g_strdup (size);
}

const gchar *
app_info_get_uid (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), NULL);
  return self->uid;
}

void
app_info_set_uid (AppInfo     *self,
                  const gchar *uid)
{
  g_return_if_fail (APP_IS_INFO (self));
  g_free (self->uid);
  self->uid = g_strdup (uid);
}

const gchar *
app_info_get_install_date (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), NULL);
  return self->install_date;
}

void
app_info_set_install_date (AppInfo     *self,
                           const gchar *install_date)
{
  g_return_if_fail (APP_IS_INFO (self));
  g_free (self->install_date);
  self->install_date = g_strdup (install_date);
}

gboolean
app_info_get_is_enabled (AppInfo *self)
{
  g_return_val_if_fail (APP_IS_INFO (self), TRUE); /* Default to TRUE */
  return self->is_enabled;
}

void
app_info_set_is_enabled (AppInfo *self,
                         gboolean is_enabled)
{
  g_return_if_fail (APP_IS_INFO (self));
  self->is_enabled = is_enabled;
}
