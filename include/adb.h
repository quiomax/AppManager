#pragma once

#include <glib.h>
#include "app.h"

G_BEGIN_DECLS

/**
 * adb_check_presence:
 *
 * Sistemde veya tanımlı yollarda ADB aracının olup olmadığını kontrol eder.
 *
 * Returns: ADB bulunduysa TRUE, aksi halde FALSE.
 */
gboolean adb_check_presence (void);

/**
 * AdbDevice:
 * @serial: Cihaz seri numarası
 * @model: Cihaz modeli (ör. "Pixel_6")
 * @is_authorized: Cihazın yetkili olup olmadığı
 */
typedef struct {
  gchar *serial;
  gchar *model;
  gboolean is_authorized;
} AdbDevice;

/**
 * adb_device_free:
 * @device: Serbest bırakılacak AdbDevice
 */
void adb_device_free (AdbDevice *device);

/**
 * adb_get_devices:
 * @error: Hata dönüşü için
 *
 * Bağlı cihazların listesini döndürür.
 *
 * Returns: (element-type AdbDevice) (transfer full): AdbDevice listesi.
 */
GList *adb_get_devices (GError **error);

/**
 * adb_get_packages:
 * @serial: Cihaz seri numarası
 * @flags: pm list packages komutuna eklenecek bayraklar (ör. "-3", "-s")
 * @type: Oluşturulacak AppInfo nesnelerine atanacak tür
 * @error: Hata dönüşü için
 *
 * Belirtilen cihazdaki paketleri listeler.
 *
 * Returns: (element-type AppInfo) (transfer full): AppInfo listesi.
 */
GList *adb_get_packages (const gchar *serial, const gchar *flags, AppType type, GError **error);

/**
 * adb_get_package_details:
 * @serial: Cihaz seri numarası
 * @app: Detayları alınacak AppInfo nesnesi
 * @error: Hata dönüşü için
 *
 * Belirtilen cihazdaki bir paketin detaylarını alır.
 *
 * Returns: Başarılı olursa TRUE, aksi halde FALSE.
 */
gboolean adb_get_package_details (const gchar *serial, AppInfo *app, GError **error);

G_END_DECLS
