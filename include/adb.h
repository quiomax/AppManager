#pragma once

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

/* Cihaz Bilgisi */
typedef struct {
  gchar *serial;
  gchar *model;
  gboolean is_authorized;
} AdbDevice;

/* Paket Detayları */
typedef struct {
  gchar *package_name;
  gchar *version;
  gchar *size;
  gchar *uid;
  gchar *install_date;
} AdbPackageDetails;

/* Yardımcı Fonksiyonlar */
void adb_device_free (AdbDevice *device);
void adb_package_details_free (AdbPackageDetails *details);
gboolean adb_check_presence (void);

/* Asenkron Cihaz Listeleme */
void adb_get_devices_async (GCancellable        *cancellable,
                            GAsyncReadyCallback  callback,
                            gpointer             user_data);

GList *adb_get_devices_finish (GAsyncResult  *result,
                               GError       **error);

/* Asenkron Paket Listeleme */
/* Dönüş: GList* (gchar* package_name) */
void adb_get_packages_async (const gchar         *serial,
                             const gchar         *flags,
                             GCancellable        *cancellable,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data);

GList *adb_get_packages_finish (GAsyncResult  *result,
                                GError       **error);

/* Asenkron Paket Detayları */
void adb_get_package_details_async (const gchar         *serial,
                                    const gchar         *package_name,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data);

AdbPackageDetails *adb_get_package_details_finish (GAsyncResult  *result,
                                                   GError       **error);

G_END_DECLS
