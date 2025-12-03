#pragma once

#include "app.h"
#include <glib.h>

G_BEGIN_DECLS

/**
 * utils_load_app_names:
 *
 * data/AppNames.txt dosyasını okur ve belleğe yükler.
 * Bu fonksiyon program başlangıcında çağrılmalıdır.
 */
void utils_load_app_names (void);

/**
 * utils_load_app_categories:
 *
 * MaliciousApps.txt ve SafeApps.txt dosyalarını okur ve belleğe yükler.
 * Bu fonksiyon program başlangıcında çağrılmalıdır.
 */
void utils_load_app_categories (void);

/**
 * utils_reload_app_data:
 *
 * Bellekteki uygulama isimlerini ve kategorilerini temizler.
 * Bir sonraki erişimde dosyadan tekrar okunmasını sağlar.
 */
void utils_reload_app_data (void);

/**
 * utils_get_app_name:
 * @package_name: Paket adı (örn. "com.whatsapp")
 *
 * Verilen paket adı için kullanıcı dostu ismi döndürür.
 * Eğer eşleşme bulunamazsa, paket adından türetilen bir isim döndürür.
 *
 * Returns: (transfer full): Uygulama adı.
 */
gchar *utils_get_app_name (const gchar *package_name);

/**
 * utils_get_app_category:
 * @package_name: Paket adı (örn. "com.whatsapp")
 *
 * Verilen paket adı için kategoriyi döndürür.
 *
 * Returns: Uygulama kategorisi (Zararlı/Güvenli/Bilinmeyen).
 */
AppCategory utils_get_app_category (const gchar *package_name);

/**
 * utils_save_app_category:
 * @package_name: Paket adı
 * @old_category: Eski kategori
 * @new_category: Yeni kategori
 *
 * Uygulamanın kategorisini değiştirir ve dosyalara kaydeder.
 */
void utils_save_app_category (const gchar *package_name,
                              AppCategory old_category,
                              AppCategory new_category);

G_END_DECLS
