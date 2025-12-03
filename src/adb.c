#include "adb.h"
#include <string.h>

/* --- Yardımcılar --- */

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
  g_free (device->manufacturer);
  g_free (device->brand);
  g_free (device->name);
  g_free (device);
}

void
adb_package_details_free (AdbPackageDetails *details)
{
  if (!details)
    return;
  g_free (details->package_name);
  g_free (details->version);
  g_free (details->size);
  g_free (details->uid);
  g_free (details->install_date);
  g_free (details);
}

void
adb_operation_result_free (AdbOperationResult *result)
{
  if (!result)
    return;
  g_free (result->message);
  g_free (result->error_output);
  g_free (result);
}

static AdbDevice *
parse_device_line (const gchar *line_orig)
{
  gchar **parts;
  AdbDevice *device;
  guint i;

  gchar *line = g_strdup (line_orig);
  g_strstrip (line);

  if (g_str_equal (line, "")
      || g_str_has_prefix (line, "List of devices attached")
      || g_str_has_prefix (line, "*") || g_str_has_prefix (line, "adb server"))
    {
      g_free (line);
      return NULL;
    }

  parts = g_strsplit (line, " ", -1);
  if (!parts)
    {
      g_free (line);
      return NULL;
    }

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
        device->is_authorized = FALSE;
      else if (g_str_equal (parts[i], "device"))
        device->is_authorized = TRUE;
      else if (g_str_has_prefix (parts[i], "model:"))
        device->model = g_strdup (parts[i] + 6);
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

/* --- Cihaz Özelliklerini Alma --- */

static gchar *
get_device_property (const gchar *serial, const gchar *property,
                     GError **error)
{
  g_autofree gchar *adb_path = get_adb_path ();
  g_autoptr (GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr (GDataInputStream) data_stream = NULL;
  gchar *line = NULL;
  gchar *result = NULL;

  if (!adb_path)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "ADB bulunamadı");
      return NULL;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, error, adb_path,
                           "-s", serial, "shell", "getprop", property, NULL);

  if (!proc)
    {
      /* Hata varsa NULL döndür */
      return NULL;
    }

  /* Hata kontrolü yap */
  if (g_subprocess_get_stderr_pipe (proc))
    {
      /* Hata varsa stderr'den oku ve ignore et */
    }

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL);
  if (line)
    {
      g_strstrip (line);
      if (g_str_has_prefix (line, "[") && g_str_has_suffix (line, "]"))
        {
          /* [value] formatından value'yu al */
          gchar *start = line + 1;
          gchar *end = strrchr (line, ']');
          if (end && end > start)
            {
              *end = '\0';
              result = g_strdup (start);
            }
        }
      else
        {
          result = g_strdup (line);
        }
      g_free (line);
    }

  /* Eğer sonuç NULL veya boşsa, boş string döndür */
  if (!result)
    result = g_strdup ("");

  return result;
}

/* --- Asenkron Cihaz Listeleme --- */

static void
adb_get_devices_thread (GTask *task, gpointer source_object G_GNUC_UNUSED,
                        gpointer task_data G_GNUC_UNUSED,
                        GCancellable *cancellable)
{
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr (GDataInputStream) data_stream = NULL;
  GList *devices = NULL;
  gchar *line = NULL;

  if (!adb_path)
    {
      g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                               "ADB bulunamadı");
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, adb_path,
                           "devices", "-l", NULL);

  if (!proc)
    {
      g_task_return_error (task, error);
      return;
    }

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line
          = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL))
         != NULL)
    {
      if (g_cancellable_is_cancelled (cancellable))
        {
          g_free (line);
          break;
        }

      AdbDevice *device = parse_device_line (line);
      if (device)
        {
          /* Yetkisiz cihazlar için boş değerler atayalım */
          device->manufacturer = g_strdup ("");
          device->brand = g_strdup ("");
          device->name = g_strdup ("");

          /* Sadece yetkili cihazlar için ekstra bilgileri al */
          if (device->is_authorized)
            {
              /* Ekstra cihaz bilgilerini al */
              GError *prop_error = NULL;

              g_free (device->manufacturer);
              device->manufacturer = get_device_property (
                  device->serial, "ro.product.manufacturer", &prop_error);
              if (prop_error)
                {
                  g_warning ("Manufacturer alınamadı: %s",
                             prop_error->message);
                  g_clear_error (&prop_error);
                }

              g_free (device->brand);
              device->brand = get_device_property (
                  device->serial, "ro.product.brand", &prop_error);
              if (prop_error)
                {
                  g_warning ("Brand alınamadı: %s", prop_error->message);
                  g_clear_error (&prop_error);
                }

              g_free (device->name);
              device->name = get_device_property (
                  device->serial, "ro.product.name", &prop_error);
              if (prop_error)
                {
                  g_warning ("Name alınamadı: %s", prop_error->message);
                  g_clear_error (&prop_error);
                }
            }

          devices = g_list_append (devices, device);
        }
      g_free (line);
    }

  g_task_return_pointer (task, devices, (GDestroyNotify)g_list_free);
}

void
adb_get_devices_async (GCancellable *cancellable, GAsyncReadyCallback callback,
                       gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  g_task_run_in_thread (task, adb_get_devices_thread);
  g_object_unref (task);
}

GList *
adb_get_devices_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Paket Listeleme --- */

typedef struct
{
  gchar *serial;
  gchar *flags;
} GetPackagesData;

static void
get_packages_data_free (GetPackagesData *data)
{
  g_free (data->serial);
  g_free (data->flags);
  g_free (data);
}

static void
adb_get_packages_thread (GTask *task, gpointer source_object G_GNUC_UNUSED,
                         gpointer task_data, GCancellable *cancellable)
{
  GetPackagesData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr (GDataInputStream) data_stream = NULL;
  GList *packages = NULL; /* List of strings */
  gchar *line = NULL;

  if (!adb_path)
    {
      g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                               "ADB bulunamadı");
      return;
    }

  GPtrArray *args = g_ptr_array_new ();
  g_ptr_array_add (args, adb_path);
  g_ptr_array_add (args, "-s");
  g_ptr_array_add (args, data->serial);
  g_ptr_array_add (args, "shell");
  g_ptr_array_add (args, "pm");
  g_ptr_array_add (args, "list");
  g_ptr_array_add (args, "packages");
  if (data->flags && data->flags[0])
    g_ptr_array_add (args, data->flags);
  g_ptr_array_add (args, NULL);

  proc = g_subprocess_newv ((const gchar *const *)args->pdata,
                            G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error);
  g_ptr_array_free (args, TRUE);

  if (!proc)
    {
      g_task_return_error (task, error);
      return;
    }

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line
          = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL))
         != NULL)
    {
      if (g_cancellable_is_cancelled (cancellable))
        {
          g_free (line);
          break;
        }

      g_strstrip (line);
      if (g_str_has_prefix (line, "package:"))
        {
          gchar *pkg_name = g_strdup (line + 8);
          packages = g_list_append (packages, pkg_name);
        }
      g_free (line);
    }

  g_task_return_pointer (task, packages, (GDestroyNotify)g_list_free);
}

void
adb_get_packages_async (const gchar *serial, const gchar *flags,
                        GCancellable *cancellable,
                        GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  GetPackagesData *data = g_new0 (GetPackagesData, 1);
  data->serial = g_strdup (serial);
  data->flags = g_strdup (flags);
  g_task_set_task_data (task, data, (GDestroyNotify)get_packages_data_free);
  g_task_run_in_thread (task, adb_get_packages_thread);
  g_object_unref (task);
}

GList *
adb_get_packages_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Paket Detayları --- */

typedef struct
{
  gchar *serial;
  gchar *package_name;
} GetDetailsData;

static void
get_details_data_free (GetDetailsData *data)
{
  g_free (data->serial);
  g_free (data->package_name);
  g_free (data);
}

static void
adb_get_package_details_thread (GTask *task,
                                gpointer source_object G_GNUC_UNUSED,
                                gpointer task_data, GCancellable *cancellable)
{
  GetDetailsData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr (GDataInputStream) data_stream = NULL;
  AdbPackageDetails *details;
  gchar *line = NULL;

  if (!adb_path)
    {
      g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                               "ADB bulunamadı");
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, adb_path,
                           "-s", data->serial, "shell", "dumpsys", "package",
                           data->package_name, NULL);

  if (!proc)
    {
      g_task_return_error (task, error);
      return;
    }

  details = g_new0 (AdbPackageDetails, 1);
  details->package_name = g_strdup (data->package_name);
  details->size = g_strdup ("Hesaplanıyor..."); /* Placeholder */

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line
          = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL))
         != NULL)
    {
      if (g_cancellable_is_cancelled (cancellable))
        {
          g_free (line);
          break;
        }

      gchar *trimmed = g_strstrip (line);

      if (g_str_has_prefix (trimmed, "versionName="))
        details->version = g_strdup (trimmed + 12);
      else if (g_str_has_prefix (trimmed, "userId="))
        details->uid = g_strdup (trimmed + 7);
      else if (g_str_has_prefix (trimmed, "appId=") && details->uid == NULL)
        details->uid = g_strdup (trimmed + 6);
      else if (g_str_has_prefix (trimmed, "firstInstallTime="))
        details->install_date = g_strdup (trimmed + 17);
      else if (strstr (trimmed, "User 0:"))
        {
          /* "User 0: ... enabled=0 ..." or similar */
          /* This line is complex, let's look for "enabled=" */
          gchar *enabled_pos = strstr (trimmed, "enabled=");
          if (enabled_pos)
            {
              int state = atoi (enabled_pos + 8);
              /* 0: DEFAULT (Enabled), 1: ENABLED, 2: DISABLED, 3:
               * DISABLED_USER, 4: DISABLED_UNTIL_USED */
              /* We consider 0 and 1 as enabled, others as disabled */
              details->is_enabled = (state == 0 || state == 1);
            }
        }

      g_free (line);
    }

  g_task_return_pointer (task, details,
                         (GDestroyNotify)adb_package_details_free);
}

void
adb_get_package_details_async (const gchar *serial, const gchar *package_name,
                               GCancellable *cancellable,
                               GAsyncReadyCallback callback,
                               gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  GetDetailsData *data = g_new0 (GetDetailsData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  g_task_set_task_data (task, data, (GDestroyNotify)get_details_data_free);
  g_task_run_in_thread (task, adb_get_package_details_thread);
  g_object_unref (task);
}

AdbPackageDetails *
adb_get_package_details_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Uygulama Kaldırma --- */

typedef struct
{
  gchar *serial;
  gchar *package_name;
} UninstallPackageData;

static void
uninstall_package_data_free (UninstallPackageData *data)
{
  g_free (data->serial);
  g_free (data->package_name);
  g_free (data);
}

static void
adb_uninstall_package_thread (GTask *task,
                              gpointer source_object G_GNUC_UNUSED,
                              gpointer task_data, GCancellable *cancellable)
{
  UninstallPackageData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr (GDataInputStream) data_stream = NULL;
  AdbOperationResult *result = g_new0 (AdbOperationResult, 1);
  gchar *line = NULL;

  if (!adb_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("ADB bulunamadı");
      result->error_output
          = g_strdup ("ADB executable not found in PATH or tools directory");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  proc = g_subprocess_new (
      G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE, &error,
      adb_path, "-s", data->serial, "uninstall", data->package_name, NULL);

  if (!proc)
    {
      result->success = FALSE;
      result->message = g_strdup ("Komut çalıştırılamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  /* Wait for the process to complete */
  if (!g_subprocess_wait_check (proc, cancellable, &error))
    {
      result->success = FALSE;
      result->message = g_strdup ("İşlem başarısız oldu");
      result->error_output
          = g_strdup (error ? error->message : "Bilinmeyen hata");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_clear_error (&error);
      return;
    }

  /* Read stdout to get the result */
  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  GString *output = g_string_new ("");
  while ((line
          = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL))
         != NULL)
    {
      if (g_cancellable_is_cancelled (cancellable))
        {
          g_free (line);
          break;
        }
      g_string_append (output, line);
      g_string_append_c (output, '\n');
      g_free (line);
    }

  /* Read stderr for error messages */
  GInputStream *stderr_stream = g_subprocess_get_stderr_pipe (proc);
  if (stderr_stream)
    {
      g_autoptr (GDataInputStream) stderr_data_stream
          = g_data_input_stream_new (stderr_stream);
      gchar *stderr_line = NULL;
      GString *stderr_output = g_string_new ("");

      while ((stderr_line = g_data_input_stream_read_line_utf8 (
                  stderr_data_stream, NULL, NULL, NULL))
             != NULL)
        {
          g_string_append (stderr_output, stderr_line);
          g_string_append_c (stderr_output, '\n');
          g_free (stderr_line);
        }

      if (stderr_output->len > 0)
        result->error_output = g_strdup (stderr_output->str);
      g_string_free (stderr_output, TRUE);
    }

  /* Check if uninstall was successful */
  if (g_subprocess_get_exit_status (proc) == 0)
    {
      result->success = TRUE;
      result->message = g_strdup ("Uygulama başarıyla kaldırıldı");
    }
  else
    {
      result->success = FALSE;
      result->message = g_strdup ("Uygulama kaldırılamadı");
      if (!result->error_output)
        result->error_output = g_strdup (output->str);
    }

  g_string_free (output, TRUE);
  g_task_return_pointer (task, result,
                         (GDestroyNotify)adb_operation_result_free);
}

void
adb_uninstall_package_async (const gchar *serial, const gchar *package_name,
                             GCancellable *cancellable,
                             GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  UninstallPackageData *data = g_new0 (UninstallPackageData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  g_task_set_task_data (task, data,
                        (GDestroyNotify)uninstall_package_data_free);
  g_task_run_in_thread (task, adb_uninstall_package_thread);
  g_object_unref (task);
}

AdbOperationResult *
adb_uninstall_package_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Uygulama Yedekleme --- */

typedef struct
{
  gchar *serial;
  gchar *package_name;
  gchar *output_path;
} BackupPackageData;

static void
backup_package_data_free (BackupPackageData *data)
{
  g_free (data->serial);
  g_free (data->package_name);
  g_free (data->output_path);
  g_free (data);
}

static void
adb_backup_package_thread (GTask *task, gpointer source_object G_GNUC_UNUSED,
                           gpointer task_data, GCancellable *cancellable)
{
  BackupPackageData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc_path = NULL;
  g_autoptr (GSubprocess) proc_pull = NULL;
  AdbOperationResult *result = g_new0 (AdbOperationResult, 1);
  gchar *path_line = NULL;
  gchar *remote_apk_path = NULL;

  if (!adb_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("ADB bulunamadı");
      result->error_output
          = g_strdup ("ADB executable not found in PATH or tools directory");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  /* 1. Get APK path */
  proc_path = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error,
                                adb_path, "-s", data->serial, "shell", "pm",
                                "path", data->package_name, NULL);

  if (!proc_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("APK yolu bulunamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  GInputStream *stdout_stream = g_subprocess_get_stdout_pipe (proc_path);
  g_autoptr (GDataInputStream) data_stream
      = g_data_input_stream_new (stdout_stream);

  path_line
      = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL);

  if (!path_line || !g_str_has_prefix (path_line, "package:"))
    {
      result->success = FALSE;
      result->message = g_strdup ("APK yolu alınamadı");
      result->error_output
          = g_strdup ("pm path command failed or returned unexpected output");
      g_free (path_line);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  /* "package:/data/app/..." -> "/data/app/..." */
  remote_apk_path = g_strdup (path_line + 8);
  g_strstrip (remote_apk_path);
  g_free (path_line);

  /* 2. Pull APK */
  proc_pull = g_subprocess_new (G_SUBPROCESS_FLAGS_NONE, &error, adb_path,
                                "-s", data->serial, "pull", remote_apk_path,
                                data->output_path, NULL);

  g_free (remote_apk_path);

  if (!proc_pull)
    {
      result->success = FALSE;
      result->message = g_strdup ("Yedekleme komutu çalıştırılamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  /* Wait for the process to complete */
  if (!g_subprocess_wait_check (proc_pull, cancellable, &error))
    {
      result->success = FALSE;
      result->message = g_strdup ("Yedekleme işlemi başarısız oldu");
      result->error_output
          = g_strdup (error ? error->message : "Bilinmeyen hata");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_clear_error (&error);
      return;
    }

  /* Check if backup was successful */
  if (g_subprocess_get_exit_status (proc_pull) == 0)
    {
      result->success = TRUE;
      result->message = g_strdup_printf ("%s dosyasına yedekleme tamamlandı",
                                         data->output_path);
    }
  else
    {
      result->success = FALSE;
      result->message = g_strdup ("Yedekleme işlemi başarısız oldu");
      result->error_output
          = g_strdup_printf ("Yedekleme işlemi başarısız oldu, çıkış kodu: %d",
                             g_subprocess_get_exit_status (proc_pull));
    }

  g_task_return_pointer (task, result,
                         (GDestroyNotify)adb_operation_result_free);
}

void
adb_backup_package_async (const gchar *serial, const gchar *package_name,
                          const gchar *output_path, GCancellable *cancellable,
                          GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  BackupPackageData *data = g_new0 (BackupPackageData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  data->output_path = g_strdup (output_path);
  g_task_set_task_data (task, data, (GDestroyNotify)backup_package_data_free);
  g_task_run_in_thread (task, adb_backup_package_thread);
  g_object_unref (task);
}

AdbOperationResult *
adb_backup_package_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Uygulama Kurma --- */

typedef struct
{
  gchar *serial;
  gchar *apk_path;
} InstallPackageData;

static void
install_package_data_free (InstallPackageData *data)
{
  g_free (data->serial);
  g_free (data->apk_path);
  g_free (data);
}

static void
adb_install_package_thread (GTask *task, gpointer source_object G_GNUC_UNUSED,
                            gpointer task_data, GCancellable *cancellable)
{
  InstallPackageData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  AdbOperationResult *result = g_new0 (AdbOperationResult, 1);

  if (!adb_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("ADB bulunamadı");
      result->error_output
          = g_strdup ("ADB executable not found in PATH or tools directory");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_NONE, &error, adb_path, "-s",
                           data->serial, "install", data->apk_path, NULL);

  if (!proc)
    {
      result->success = FALSE;
      result->message = g_strdup ("Kurulum komutu çalıştırılamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  /* Wait for the process to complete */
  if (!g_subprocess_wait_check (proc, cancellable, &error))
    {
      result->success = FALSE;
      result->message = g_strdup ("Kurulum işlemi başarısız oldu");
      result->error_output
          = g_strdup (error ? error->message : "Bilinmeyen hata");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_clear_error (&error);
      return;
    }

  /* Check if installation was successful */
  if (g_subprocess_get_exit_status (proc) == 0)
    {
      result->success = TRUE;
      result->message = g_strdup ("Uygulama başarıyla kuruldu");
    }
  else
    {
      result->success = FALSE;
      result->message = g_strdup ("Uygulama kurulamadı");
      result->error_output
          = g_strdup_printf ("Kurulum işlemi başarısız oldu, çıkış kodu: %d",
                             g_subprocess_get_exit_status (proc));
    }

  g_task_return_pointer (task, result,
                         (GDestroyNotify)adb_operation_result_free);
}

void
adb_install_package_async (const gchar *serial, const gchar *apk_path,
                           GCancellable *cancellable,
                           GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  InstallPackageData *data = g_new0 (InstallPackageData, 1);
  data->serial = g_strdup (serial);
  data->apk_path = g_strdup (apk_path);
  g_task_set_task_data (task, data, (GDestroyNotify)install_package_data_free);
  g_task_run_in_thread (task, adb_install_package_thread);
  g_object_unref (task);
}

AdbOperationResult *
adb_install_package_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Uygulama Dondurma --- */

typedef struct
{
  gchar *serial;
  gchar *package_name;
} FreezePackageData;

static void
freeze_package_data_free (FreezePackageData *data)
{
  g_free (data->serial);
  g_free (data->package_name);
  g_free (data);
}

static void
adb_freeze_package_thread (GTask *task, gpointer source_object G_GNUC_UNUSED,
                           gpointer task_data, GCancellable *cancellable)
{
  FreezePackageData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  AdbOperationResult *result = g_new0 (AdbOperationResult, 1);

  if (!adb_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("ADB bulunamadı");
      result->error_output
          = g_strdup ("ADB executable not found in PATH or tools directory");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_NONE, &error, adb_path, "-s",
                           data->serial, "shell", "pm", "disable-user",
                           data->package_name, NULL);

  if (!proc)
    {
      result->success = FALSE;
      result->message = g_strdup ("Dondurma komutu çalıştırılamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  /* Wait for the process to complete */
  if (!g_subprocess_wait_check (proc, cancellable, &error))
    {
      result->success = FALSE;
      result->message = g_strdup ("Dondurma işlemi başarısız oldu");
      result->error_output
          = g_strdup (error ? error->message : "Bilinmeyen hata");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_clear_error (&error);
      return;
    }

  /* Check if freeze was successful */
  if (g_subprocess_get_exit_status (proc) == 0)
    {
      result->success = TRUE;
      result->message = g_strdup ("Uygulama başarıyla donduruldu");
    }
  else
    {
      result->success = FALSE;
      result->message = g_strdup ("Uygulama dondurulamadı");
      result->error_output
          = g_strdup_printf ("Dondurma işlemi başarısız oldu, çıkış kodu: %d",
                             g_subprocess_get_exit_status (proc));
    }

  g_task_return_pointer (task, result,
                         (GDestroyNotify)adb_operation_result_free);
}

void
adb_freeze_package_async (const gchar *serial, const gchar *package_name,
                          GCancellable *cancellable,
                          GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  FreezePackageData *data = g_new0 (FreezePackageData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  g_task_set_task_data (task, data, (GDestroyNotify)freeze_package_data_free);
  g_task_run_in_thread (task, adb_freeze_package_thread);
  g_object_unref (task);
}

AdbOperationResult *
adb_freeze_package_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Uygulama Etkinleştirme --- */

typedef struct
{
  gchar *serial;
  gchar *package_name;
} UnfreezePackageData;

static void
unfreeze_package_data_free (UnfreezePackageData *data)
{
  g_free (data->serial);
  g_free (data->package_name);
  g_free (data);
}

static void
adb_unfreeze_package_thread (GTask *task, gpointer source_object G_GNUC_UNUSED,
                             gpointer task_data, GCancellable *cancellable)
{
  UnfreezePackageData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  AdbOperationResult *result = g_new0 (AdbOperationResult, 1);

  if (!adb_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("ADB bulunamadı");
      result->error_output
          = g_strdup ("ADB executable not found in PATH or tools directory");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_NONE, &error, adb_path, "-s",
                           data->serial, "shell", "pm", "enable",
                           data->package_name, NULL);

  if (!proc)
    {
      result->success = FALSE;
      result->message = g_strdup ("Etkinleştirme komutu çalıştırılamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  /* Wait for the process to complete */
  if (!g_subprocess_wait_check (proc, cancellable, &error))
    {
      result->success = FALSE;
      result->message = g_strdup ("Etkinleştirme işlemi başarısız oldu");
      result->error_output
          = g_strdup (error ? error->message : "Bilinmeyen hata");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_clear_error (&error);
      return;
    }

  /* Check if unfreeze was successful */
  if (g_subprocess_get_exit_status (proc) == 0)
    {
      result->success = TRUE;
      result->message = g_strdup ("Uygulama başarıyla açıldı");
    }
  else
    {
      result->success = FALSE;
      result->message = g_strdup ("Uygulama açılamadı");
      result->error_output = g_strdup_printf (
          "Etkinleştirme işlemi başarısız oldu, çıkış kodu: %d",
          g_subprocess_get_exit_status (proc));
    }

  g_task_return_pointer (task, result,
                         (GDestroyNotify)adb_operation_result_free);
}

void
adb_unfreeze_package_async (const gchar *serial, const gchar *package_name,
                            GCancellable *cancellable,
                            GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  UnfreezePackageData *data = g_new0 (UnfreezePackageData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  g_task_set_task_data (task, data,
                        (GDestroyNotify)unfreeze_package_data_free);
  g_task_run_in_thread (task, adb_unfreeze_package_thread);
  g_object_unref (task);
}

AdbOperationResult *
adb_unfreeze_package_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Uygulama Veri Temizleme --- */

typedef struct
{
  gchar *serial;
  gchar *package_name;
} ClearPackageDataData;

static void
clear_package_data_data_free (ClearPackageDataData *data)
{
  g_free (data->serial);
  g_free (data->package_name);
  g_free (data);
}

static void
adb_clear_package_data_thread (GTask *task,
                               gpointer source_object G_GNUC_UNUSED,
                               gpointer task_data, GCancellable *cancellable)
{
  ClearPackageDataData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr (GSubprocess) proc = NULL;
  AdbOperationResult *result = g_new0 (AdbOperationResult, 1);

  if (!adb_path)
    {
      result->success = FALSE;
      result->message = g_strdup ("ADB bulunamadı");
      result->error_output
          = g_strdup ("ADB executable not found in PATH or tools directory");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_NONE, &error, adb_path, "-s",
                           data->serial, "shell", "pm", "clear",
                           data->package_name, NULL);

  if (!proc)
    {
      result->success = FALSE;
      result->message = g_strdup ("Veri temizleme komutu çalıştırılamadı");
      result->error_output = g_strdup (error->message);
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_error_free (error);
      return;
    }

  /* Wait for the process to complete */
  if (!g_subprocess_wait_check (proc, cancellable, &error))
    {
      result->success = FALSE;
      result->message = g_strdup ("Veri temizleme işlemi başarısız oldu");
      result->error_output
          = g_strdup (error ? error->message : "Bilinmeyen hata");
      g_task_return_pointer (task, result,
                             (GDestroyNotify)adb_operation_result_free);
      g_clear_error (&error);
      return;
    }

  /* Check if clear was successful */
  if (g_subprocess_get_exit_status (proc) == 0)
    {
      result->success = TRUE;
      result->message = g_strdup ("Uygulama verileri başarıyla temizlendi");
    }
  else
    {
      result->success = FALSE;
      result->message = g_strdup ("Uygulama verileri temizlenemedi");
      result->error_output = g_strdup_printf (
          "Veri temizleme işlemi başarısız oldu, çıkış kodu: %d",
          g_subprocess_get_exit_status (proc));
    }

  g_task_return_pointer (task, result,
                         (GDestroyNotify)adb_operation_result_free);
}

void
adb_clear_package_data_async (const gchar *serial, const gchar *package_name,
                              GCancellable *cancellable,
                              GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  ClearPackageDataData *data = g_new0 (ClearPackageDataData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  g_task_set_task_data (task, data,
                        (GDestroyNotify)clear_package_data_data_free);
  g_task_run_in_thread (task, adb_clear_package_data_thread);
  g_object_unref (task);
}

AdbOperationResult *
adb_clear_package_data_finish (GAsyncResult *result, GError **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}
