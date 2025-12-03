#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static GHashTable *app_names_table = NULL;

void
utils_load_app_names (void)
{
  gchar *content = NULL;
  gchar **lines = NULL;
  GError *error = NULL;
  gchar *user_data_path;
  gchar *user_file_path;
  gchar *dev_file_path;
  guint i;

  if (app_names_table)
    return;

  app_names_table
      = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* Kullanıcı veri dizini */
  user_data_path
      = g_build_filename (g_get_user_data_dir (), "Muha", "AppManager", NULL);
  user_file_path = g_build_filename (user_data_path, "AppNames.txt", NULL);

  /* Geliştirme ortamı dosyası */
  dev_file_path = g_build_filename ("data", "AppNames.txt", NULL);

  /* Önce kullanıcı dizininde ara */
  if (g_file_test (user_file_path, G_FILE_TEST_EXISTS))
    {
      /* Kullanıcı dosyası var, onu kullan */
      if (!g_file_get_contents (user_file_path, &content, NULL, &error))
        {
          g_warning ("AppNames.txt okunamadı: %s", error->message);
          g_clear_error (&error);
        }
    }
  else
    {
      /* Kullanıcı dosyası yok, geliştirme dosyasından kopyala */
      if (g_file_test (dev_file_path, G_FILE_TEST_EXISTS))
        {
          gchar *dev_content = NULL;

          if (g_file_get_contents (dev_file_path, &dev_content, NULL, &error))
            {
              /* Kullanıcı dizinini oluştur */
              g_mkdir_with_parents (user_data_path, 0755);

              /* Dosyayı kullanıcı dizinine kopyala */
              if (!g_file_set_contents (user_file_path, dev_content, -1,
                                        &error))
                {
                  g_warning (
                      "AppNames.txt kullanıcı dizinine kopyalanamadı: %s",
                      error->message);
                  g_clear_error (&error);
                }
              else
                {
                  g_message ("AppNames.txt kullanıcı dizinine kopyalandı: %s",
                             user_file_path);
                }

              /* Geliştirme dosyasını kullan */
              content = dev_content;
            }
          else
            {
              g_warning ("Geliştirme AppNames.txt okunamadı: %s",
                         error->message);
              g_clear_error (&error);
            }
        }
      else
        {
          /* Hiçbir dosya yok, boş bir dosya oluştur */
          const gchar *default_content
              = "# Uygulama İsim Eşlemeleri\n"
                "# Format: package_name=Display Name\n"
                "# Bu dosyayı düzenleyerek uygulama isimlerini "
                "özelleştirebilirsiniz\n\n";

          g_mkdir_with_parents (user_data_path, 0755);

          if (!g_file_set_contents (user_file_path, default_content, -1,
                                    &error))
            {
              g_warning ("Varsayılan AppNames.txt oluşturulamadı: %s",
                         error->message);
              g_clear_error (&error);
            }
          else
            {
              g_message ("Varsayılan AppNames.txt oluşturuldu: %s",
                         user_file_path);
            }
        }
    }

  /* İçeriği parse et */
  if (content)
    {
      lines = g_strsplit (content, "\n", -1);
      for (i = 0; lines[i] != NULL; i++)
        {
          gchar *line = g_strstrip (lines[i]);
          gchar **parts;

          if (line[0] == '#' || line[0] == '\0')
            continue;

          parts = g_strsplit (line, "=", 2);
          if (parts[0] && parts[1])
            {
              gchar *key = g_strstrip (parts[0]);
              gchar *value = g_strstrip (parts[1]);

              if (key[0] != '\0' && value[0] != '\0')
                {
                  g_hash_table_insert (app_names_table, g_strdup (key),
                                       g_strdup (value));
                }
            }
          g_strfreev (parts);
        }
      g_strfreev (lines);
      g_free (content);
    }

  g_free (user_data_path);
  g_free (user_file_path);
  g_free (dev_file_path);
}

static gchar *
format_package_name (const gchar *package_name)
{
  gchar **parts;
  gchar *result = NULL;
  guint len;

  /* com.google.android.youtube -> YouTube */
  /* com.whatsapp -> WhatsApp */

  parts = g_strsplit (package_name, ".", -1);
  len = g_strv_length (parts);

  if (len > 0)
    {
      /* Genellikle son parça veya sondan bir önceki parça anlamlıdır */
      /* Ancak "android" gibi genel kelimeleri elemek lazım */

      gchar *candidate = parts[len - 1];

      if (len > 1
          && (g_str_equal (candidate, "android")
              || g_str_equal (candidate, "app")
              || g_str_equal (candidate, "client")
              || g_str_equal (candidate, "mobile")))
        {
          candidate = parts[len - 2];
        }

      /* İlk harfi büyüt */
      result = g_strdup (candidate);
      if (result && result[0])
        result[0] = toupper (result[0]);
    }
  else
    {
      result = g_strdup (package_name);
    }

  g_strfreev (parts);
  return result;
}

gchar *
utils_get_app_name (const gchar *package_name)
{
  gchar *name;

  if (!app_names_table)
    utils_load_app_names ();

  name = g_hash_table_lookup (app_names_table, package_name);
  if (name)
    return g_strdup (name);

  return format_package_name (package_name);
}

static GHashTable *malicious_apps_set = NULL;
static GHashTable *safe_apps_set = NULL;

static void
load_app_list_file (const gchar *filename, GHashTable **set_ptr)
{
  gchar *content = NULL;
  gchar **lines = NULL;
  GError *error = NULL;
  gchar *user_data_path;
  gchar *user_file_path;
  gchar *dev_file_path;
  guint i;

  if (*set_ptr)
    return;

  *set_ptr = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  /* Kullanıcı veri dizini */
  user_data_path
      = g_build_filename (g_get_user_data_dir (), "Muha", "AppManager", NULL);
  user_file_path = g_build_filename (user_data_path, filename, NULL);

  /* Geliştirme ortamı dosyası */
  dev_file_path = g_build_filename ("data", filename, NULL);

  /* Önce kullanıcı dizininde ara */
  if (g_file_test (user_file_path, G_FILE_TEST_EXISTS))
    {
      if (!g_file_get_contents (user_file_path, &content, NULL, &error))
        {
          g_warning ("%s okunamadı: %s", filename, error->message);
          g_clear_error (&error);
        }
    }
  else
    {
      /* Kullanıcı dosyası yok, geliştirme dosyasından kopyala */
      if (g_file_test (dev_file_path, G_FILE_TEST_EXISTS))
        {
          gchar *dev_content = NULL;

          if (g_file_get_contents (dev_file_path, &dev_content, NULL, &error))
            {
              g_mkdir_with_parents (user_data_path, 0755);

              if (!g_file_set_contents (user_file_path, dev_content, -1,
                                        &error))
                {
                  g_warning ("%s kullanıcı dizinine kopyalanamadı: %s",
                             filename, error->message);
                  g_clear_error (&error);
                }
              else
                {
                  g_message ("%s kullanıcı dizinine kopyalandı: %s", filename,
                             user_file_path);
                }

              content = dev_content;
            }
          else
            {
              g_warning ("Geliştirme %s okunamadı: %s", filename,
                         error->message);
              g_clear_error (&error);
            }
        }
      else
        {
          /* Hiçbir dosya yok, boş bir dosya oluştur */
          const gchar *default_content
              = "# Uygulama Listesi\n"
                "# Format: Her satırda bir paket adı\n\n";

          g_mkdir_with_parents (user_data_path, 0755);

          if (!g_file_set_contents (user_file_path, default_content, -1,
                                    &error))
            {
              g_warning ("Varsayılan %s oluşturulamadı: %s", filename,
                         error->message);
              g_clear_error (&error);
            }
          else
            {
              g_message ("Varsayılan %s oluşturuldu: %s", filename,
                         user_file_path);
            }
        }
    }

  /* İçeriği parse et */
  if (content)
    {
      lines = g_strsplit (content, "\n", -1);
      for (i = 0; lines[i] != NULL; i++)
        {
          gchar *line = g_strstrip (lines[i]);

          if (line[0] == '#' || line[0] == '\0')
            continue;

          g_hash_table_add (*set_ptr, g_strdup (line));
        }
      g_strfreev (lines);
      g_free (content);
    }

  g_free (user_data_path);
  g_free (user_file_path);
  g_free (dev_file_path);
}

void
utils_load_app_categories (void)
{
  load_app_list_file ("MaliciousApps.txt", &malicious_apps_set);
  load_app_list_file ("SafeApps.txt", &safe_apps_set);
}

AppCategory
utils_get_app_category (const gchar *package_name)
{
  if (!malicious_apps_set || !safe_apps_set)
    utils_load_app_categories ();

  if (g_hash_table_contains (malicious_apps_set, package_name))
    return APP_CATEGORY_MALICIOUS;

  if (g_hash_table_contains (safe_apps_set, package_name))
    return APP_CATEGORY_SAFE;

  return APP_CATEGORY_UNKNOWN;
}

void
utils_save_app_category (const gchar *package_name, AppCategory old_category,
                         AppCategory new_category)
{
  if (!malicious_apps_set || !safe_apps_set)
    utils_load_app_categories ();

  /* Eski kategoriden kaldır */
  if (old_category == APP_CATEGORY_MALICIOUS)
    g_hash_table_remove (malicious_apps_set, package_name);
  else if (old_category == APP_CATEGORY_SAFE)
    g_hash_table_remove (safe_apps_set, package_name);

  /* Yeni kategoriye ekle */
  if (new_category == APP_CATEGORY_MALICIOUS)
    g_hash_table_add (malicious_apps_set, g_strdup (package_name));
  else if (new_category == APP_CATEGORY_SAFE)
    g_hash_table_add (safe_apps_set, g_strdup (package_name));

  /* Dosyalara kaydet */
  gchar *user_data_path
      = g_build_filename (g_get_user_data_dir (), "Muha", "AppManager", NULL);
  gchar *malicious_file
      = g_build_filename (user_data_path, "MaliciousApps.txt", NULL);
  gchar *safe_file = g_build_filename (user_data_path, "SafeApps.txt", NULL);

  /* MaliciousApps.txt dosyasını yaz */
  GString *malicious_content = g_string_new (
      "# Zararlı Uygulama Listesi\n# Format: Her satırda bir paket adı\n\n");
  GHashTableIter iter;
  gpointer key;

  g_hash_table_iter_init (&iter, malicious_apps_set);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    {
      g_string_append_printf (malicious_content, "%s\n", (gchar *)key);
    }

  g_file_set_contents (malicious_file, malicious_content->str, -1, NULL);
  g_string_free (malicious_content, TRUE);

  /* SafeApps.txt dosyasını yaz */
  GString *safe_content = g_string_new (
      "# Güvenli Uygulama Listesi\n# Format: Her satırda bir paket adı\n\n");

  g_hash_table_iter_init (&iter, safe_apps_set);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    {
      g_string_append_printf (safe_content, "%s\n", (gchar *)key);
    }

  g_file_set_contents (safe_file, safe_content->str, -1, NULL);
  g_string_free (safe_content, TRUE);

  g_free (user_data_path);
  g_free (malicious_file);
  g_free (safe_file);
}

void
utils_reload_app_data (void)
{
  if (app_names_table)
    {
      g_hash_table_destroy (app_names_table);
      app_names_table = NULL;
    }

  if (malicious_apps_set)
    {
      g_hash_table_destroy (malicious_apps_set);
      malicious_apps_set = NULL;
    }

  if (safe_apps_set)
    {
      g_hash_table_destroy (safe_apps_set);
      safe_apps_set = NULL;
    }
}
