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

static AdbDevice *
parse_device_line (const gchar *line_orig)
{
  gchar **parts;
  AdbDevice *device;
  guint i;

  gchar *line = g_strdup (line_orig);
  g_strstrip (line);

  if (g_str_equal (line, "") ||
      g_str_has_prefix (line, "List of devices attached") ||
      g_str_has_prefix (line, "*") ||
      g_str_has_prefix (line, "adb server"))
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

/* --- Asenkron Cihaz Listeleme --- */

static void
adb_get_devices_thread (GTask        *task,
                        gpointer      source_object G_GNUC_UNUSED,
                        gpointer      task_data G_GNUC_UNUSED,
                        GCancellable *cancellable)
{
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr(GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr(GDataInputStream) data_stream = NULL;
  GList *devices = NULL;
  gchar *line = NULL;

  if (!adb_path)
    {
      g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "ADB bulunamadı");
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error,
                           adb_path, "devices", "-l", NULL);

  if (!proc)
    {
      g_task_return_error (task, error);
      return;
    }

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL)) != NULL)
    {
      if (g_cancellable_is_cancelled (cancellable))
        {
          g_free (line);
          break;
        }

      AdbDevice *device = parse_device_line (line);
      if (device)
        devices = g_list_append (devices, device);
      g_free (line);
    }

  g_task_return_pointer (task, devices, (GDestroyNotify) g_list_free);
}

void
adb_get_devices_async (GCancellable        *cancellable,
                       GAsyncReadyCallback  callback,
                       gpointer             user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  g_task_run_in_thread (task, adb_get_devices_thread);
  g_object_unref (task);
}

GList *
adb_get_devices_finish (GAsyncResult  *result,
                        GError       **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Paket Listeleme --- */

typedef struct {
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
adb_get_packages_thread (GTask        *task,
                         gpointer      source_object G_GNUC_UNUSED,
                         gpointer      task_data,
                         GCancellable *cancellable)
{
  GetPackagesData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr(GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr(GDataInputStream) data_stream = NULL;
  GList *packages = NULL; /* List of strings */
  gchar *line = NULL;

  if (!adb_path)
    {
      g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "ADB bulunamadı");
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

  proc = g_subprocess_newv ((const gchar * const *)args->pdata, G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error);
  g_ptr_array_free (args, TRUE);

  if (!proc)
    {
      g_task_return_error (task, error);
      return;
    }

  stdout_stream = g_subprocess_get_stdout_pipe (proc);
  data_stream = g_data_input_stream_new (stdout_stream);

  while ((line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL)) != NULL)
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

  g_task_return_pointer (task, packages, (GDestroyNotify) g_list_free);
}

void
adb_get_packages_async (const gchar         *serial,
                        const gchar         *flags,
                        GCancellable        *cancellable,
                        GAsyncReadyCallback  callback,
                        gpointer             user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  GetPackagesData *data = g_new0 (GetPackagesData, 1);
  data->serial = g_strdup (serial);
  data->flags = g_strdup (flags);
  g_task_set_task_data (task, data, (GDestroyNotify) get_packages_data_free);
  g_task_run_in_thread (task, adb_get_packages_thread);
  g_object_unref (task);
}

GList *
adb_get_packages_finish (GAsyncResult  *result,
                         GError       **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

/* --- Asenkron Paket Detayları --- */

typedef struct {
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
adb_get_package_details_thread (GTask        *task,
                                gpointer      source_object G_GNUC_UNUSED,
                                gpointer      task_data,
                                GCancellable *cancellable)
{
  GetDetailsData *data = task_data;
  g_autofree gchar *adb_path = get_adb_path ();
  GError *error = NULL;
  g_autoptr(GSubprocess) proc = NULL;
  GInputStream *stdout_stream = NULL;
  g_autoptr(GDataInputStream) data_stream = NULL;
  AdbPackageDetails *details;
  gchar *line = NULL;

  if (!adb_path)
    {
      g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "ADB bulunamadı");
      return;
    }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error,
                           adb_path, "-s", data->serial, "shell", "dumpsys", "package", data->package_name,
                           NULL);

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

  while ((line = g_data_input_stream_read_line_utf8 (data_stream, NULL, NULL, NULL)) != NULL)
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

      g_free (line);
    }

  g_task_return_pointer (task, details, (GDestroyNotify) adb_package_details_free);
}

void
adb_get_package_details_async (const gchar         *serial,
                               const gchar         *package_name,
                               GCancellable        *cancellable,
                               GAsyncReadyCallback  callback,
                               gpointer             user_data)
{
  GTask *task = g_task_new (NULL, cancellable, callback, user_data);
  GetDetailsData *data = g_new0 (GetDetailsData, 1);
  data->serial = g_strdup (serial);
  data->package_name = g_strdup (package_name);
  g_task_set_task_data (task, data, (GDestroyNotify) get_details_data_free);
  g_task_run_in_thread (task, adb_get_package_details_thread);
  g_object_unref (task);
}

AdbPackageDetails *
adb_get_package_details_finish (GAsyncResult  *result,
                                GError       **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}
