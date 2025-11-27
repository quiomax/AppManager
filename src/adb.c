#include "adb.h"
#include "app.h"
#include <gio/gio.h>

static gchar *
get_adb_path (void)
{
  gchar *path;

  /* 1. Sistem yolunda ara */
  path = g_find_program_in_path ("adb");
  if (path)
    return path;

  /* 2. Çalışma dizinindeki tools klasöründe ara */
#ifdef G_OS_WIN32
  if (g_file_test ("tools/platform-tools/adb.exe", G_FILE_TEST_IS_EXECUTABLE))
    return g_strdup ("tools/platform-tools/adb.exe");
#else
  if (g_file_test ("tools/platform-tools/adb", G_FILE_TEST_IS_EXECUTABLE))
    return g_strdup ("tools/platform-tools/adb");
#endif

  return NULL;
}

gboolean
adb_get_package_details (const gchar  *serial,
                         AppInfo      *app,
                         GError      **error)
{
  g_autoptr(GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr(GDataInputStream) data_stream = NULL;
  g_autofree gchar *adb_path = get_adb_path ();
  const gchar *package_name = app_info_get_package_name (app);
  GError *local_error = NULL;
  gchar *line = NULL;

  if (!adb_path)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "ADB bulunamadı");
      return FALSE;
    }

  /* dumpsys package <package_name> komutunu çalıştır */
  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE,
                           &local_error,
                           adb_path, "-s", serial, "shell", "dumpsys", "package", package_name,
                           NULL);

  if (local_error)
    {
      g_propagate_error (error, local_error);
      return FALSE;
    }

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL)) != NULL)
    {
      gchar *trimmed = g_strstrip (line);

      // g_debug ("Dumpsys line: %s", trimmed); /* Çok fazla log üretebilir ama gerekirse açarız */

      if (g_str_has_prefix (trimmed, "versionName="))
        {
          app_info_set_version (app, trimmed + 12);
          g_debug ("Version found: %s", trimmed + 12);
        }
      else if (g_str_has_prefix (trimmed, "userId="))
        {
          app_info_set_uid (app, trimmed + 7);
          g_debug ("UID found (userId): %s", trimmed + 7);
        }
      else if (g_str_has_prefix (trimmed, "appId=") && app_info_get_uid (app) == NULL)
        {
          app_info_set_uid (app, trimmed + 6);
          g_debug ("UID found (appId): %s", trimmed + 6);
        }
      else if (g_str_has_prefix (trimmed, "firstInstallTime="))
        {
          app_info_set_install_date (app, trimmed + 17);
          g_debug ("Install date found: %s", trimmed + 17);
        }

      g_free (line);
    }

  /* Boyut bilgisi için ayrı bir komut çalıştırmak gerekebilir ama şimdilik "-" kalsın */
  app_info_set_size (app, "Hesaplanıyor...");

  return TRUE;
}

gboolean
adb_check_presence (void)
{
  g_autofree gchar *path = get_adb_path ();
  return path != NULL;
}

void
adb_device_free (AdbDevice *device)
{
  if (!device)
    return;
  g_free (device->serial);
  g_free (device->model);
  g_free (device);
}

static AdbDevice *
parse_device_line (const gchar *line_orig)
{
  gchar **parts;
  AdbDevice *device;
  guint i;

  /* Satır sonu karakterlerini ve boşlukları temizle */
  gchar *line = g_strdup (line_orig); // Create a mutable copy
  g_strstrip (line);

  if (g_str_equal (line, "") ||
      g_str_has_prefix (line, "List of devices attached") ||
      g_str_has_prefix (line, "*") ||
      g_str_has_prefix (line, "adb server"))
    {
      g_free (line);
      return NULL;
    }

  /* Boşluklara göre böl, ancak boş tokenleri atla */
  /* g_regex_split_simple daha uygun olabilir ama g_strsplit ve döngü de yeterli */
  parts = g_strsplit (line, " ", -1);
  if (!parts)
    return NULL;

  /* İlk dolu parça serial olmalı */
  device = g_new0 (AdbDevice, 1);

  for (i = 0; parts[i] != NULL; i++)
    {
      if (g_str_equal (parts[i], ""))
        continue;

      if (device->serial == NULL)
        {
          device->serial = g_strdup (parts[i]);
          continue;
        }

      if (g_str_equal (parts[i], "unauthorized"))
        {
          device->is_authorized = FALSE;
        }
      else if (g_str_equal (parts[i], "device"))
        {
          device->is_authorized = TRUE;
        }
      else if (g_str_has_prefix (parts[i], "model:"))
        {
          device->model = g_strdup (parts[i] + 6); /* "model:" uzunluğu 6 */
        }
    }

  g_strfreev (parts);
  g_free (line);

  if (!device->serial)
    {
      adb_device_free (device);
      return NULL;
    }

  if (!device->model)
    device->model = g_strdup ("Bilinmeyen Cihaz");

  return device;
}

GList *
adb_get_devices (GError **error)
{
  g_autoptr(GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr(GDataInputStream) data_stream = NULL;
  GList *devices = NULL;
  gchar *line = NULL;
  g_autofree gchar *adb_path = get_adb_path ();

  if (!adb_path)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                   "ADB aracı bulunamadı. Lütfen ADB'nin kurulu olduğundan veya tools/platform-tools klasöründe bulunduğundan emin olun.");
      return NULL;
    }

  /* TODO: ADB yolunu ayarlardan al */
  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, error,
                           adb_path, "devices", "-l", NULL);

  if (!proc)
    return NULL;

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL)) != NULL)
    {
      AdbDevice *device = parse_device_line (line);
      if (device)
        devices = g_list_append (devices, device);
      g_free (line);
    }

  return devices;
}



GList *
adb_get_packages (const gchar *serial, const gchar *flags, AppType type, GError **error)
{
  g_autoptr(GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr(GDataInputStream) data_stream = NULL;
  GList *packages = NULL;
  gchar *line = NULL;
  g_autofree gchar *adb_path = get_adb_path ();

  if (!adb_path)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "ADB bulunamadı");
      return NULL;
    }

  /* adb -s <serial> shell pm list packages <flags> */
  /* flags NULL ise boş string olarak kabul et */
  if (flags == NULL)
    flags = "";

  /* Argümanları dinamik olarak oluştur */
  /* GSubprocess varargs kullandığı için flags'i doğrudan geçemeyiz eğer boşsa */
  /* O yüzden array tabanlı constructor kullanmak daha güvenli */

  GPtrArray *args = g_ptr_array_new ();
  g_ptr_array_add (args, adb_path);
  g_ptr_array_add (args, "-s");
  g_ptr_array_add (args, (gpointer)serial);
  g_ptr_array_add (args, "shell");
  g_ptr_array_add (args, "pm");
  g_ptr_array_add (args, "list");
  g_ptr_array_add (args, "packages");

  if (flags && *flags)
    g_ptr_array_add (args, (gpointer)flags);

  g_ptr_array_add (args, NULL);

  proc = g_subprocess_newv ((const gchar * const *)args->pdata, G_SUBPROCESS_FLAGS_STDOUT_PIPE, error);
  g_ptr_array_free (args, TRUE);

  if (!proc)
    return NULL;

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL)) != NULL)
    {
      g_strstrip (line);
      if (g_str_has_prefix (line, "package:"))
        {
          /* "package:" öneki 8 karakter */
          gchar *pkg_name = g_strdup (line + 8);
          /* AppInfo oluştur */
          AppInfo *app = app_info_new (pkg_name, type);
          packages = g_list_append (packages, app);
          g_free (pkg_name);
        }
      g_free (line);
    }

  return packages;
}
