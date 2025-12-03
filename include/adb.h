#pragma once

#include <gio/gio.h>
#include <glib.h>

G_BEGIN_DECLS

/* Cihaz Bilgisi */
typedef struct
{
  gchar *serial;
  gchar *model;
  gchar *manufacturer;
  gchar *brand;
  gchar *name;
  gboolean is_authorized;
} AdbDevice;

/* Paket Detayları */
typedef struct
{
  gchar *package_name;
  gchar *version;
  gchar *size;
  gchar *uid;
  gchar *install_date;
  gboolean is_enabled;
} AdbPackageDetails;

/* İşlem Sonuçları */
typedef struct
{
  gboolean success;
  gchar *message;
  gchar *error_output;
} AdbOperationResult;

/* Yardımcı Fonksiyonlar */
void adb_device_free (AdbDevice *device);
void adb_package_details_free (AdbPackageDetails *details);
void adb_operation_result_free (AdbOperationResult *result);
gboolean adb_check_presence (void);

/* Asenkron Cihaz Listeleme */
void adb_get_devices_async (GCancellable *cancellable,
                            GAsyncReadyCallback callback, gpointer user_data);

GList *adb_get_devices_finish (GAsyncResult *result, GError **error);

/* Asenkron Paket Listeleme */
/* Dönüş: GList* (gchar* package_name) */
void adb_get_packages_async (const gchar *serial, const gchar *flags,
                             GCancellable *cancellable,
                             GAsyncReadyCallback callback, gpointer user_data);

GList *adb_get_packages_finish (GAsyncResult *result, GError **error);

/* Asenkron Paket Detayları */
void adb_get_package_details_async (const gchar *serial,
                                    const gchar *package_name,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback,
                                    gpointer user_data);

AdbPackageDetails *adb_get_package_details_finish (GAsyncResult *result,
                                                   GError **error);

/* Asenkron Uygulama İşlemleri */

/* Uygulama Kaldırma */
void adb_uninstall_package_async (const gchar *serial,
                                  const gchar *package_name,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback,
                                  gpointer user_data);

AdbOperationResult *adb_uninstall_package_finish (GAsyncResult *result,
                                                  GError **error);

/* Uygulama Yedekleme */
void adb_backup_package_async (const gchar *serial, const gchar *package_name,
                               const gchar *output_path,
                               GCancellable *cancellable,
                               GAsyncReadyCallback callback,
                               gpointer user_data);

AdbOperationResult *adb_backup_package_finish (GAsyncResult *result,
                                               GError **error);

/* Uygulama Kurma */
void adb_install_package_async (const gchar *serial, const gchar *apk_path,
                                GCancellable *cancellable,
                                GAsyncReadyCallback callback,
                                gpointer user_data);

AdbOperationResult *adb_install_package_finish (GAsyncResult *result,
                                                GError **error);

/* Uygulama Dondurma */
void adb_freeze_package_async (const gchar *serial, const gchar *package_name,
                               GCancellable *cancellable,
                               GAsyncReadyCallback callback,
                               gpointer user_data);

AdbOperationResult *adb_freeze_package_finish (GAsyncResult *result,
                                               GError **error);

/* Uygulama Etkinleştirme */
void adb_unfreeze_package_async (const gchar *serial,
                                 const gchar *package_name,
                                 GCancellable *cancellable,
                                 GAsyncReadyCallback callback,
                                 gpointer user_data);

AdbOperationResult *adb_unfreeze_package_finish (GAsyncResult *result,
                                                 GError **error);

/* Uygulama Veri Temizleme */
void adb_clear_package_data_async (const gchar *serial,
                                   const gchar *package_name,
                                   GCancellable *cancellable,
                                   GAsyncReadyCallback callback,
                                   gpointer user_data);

AdbOperationResult *adb_clear_package_data_finish (GAsyncResult *result,
                                                   GError **error);

G_END_DECLS
