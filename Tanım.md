# Proje Adı ve Geliştirici
- Proje Adı: Android App Manager (kısaca MuhaAppManager)
- Geliştirici: Muha Aliss <muhaaliss@pm.me>

# Temel Teknolojiler ve Standartlar
- Programlama Dili: C
- Kütüphaneler: Libadwaita (GTK4 tabanlı)
- Arayüz Tasarımı: GNOME Blueprint, Adwaita ögeleri öncelikli.
- Kod Standartları: GNU C dil standartları, GNOME HIG (Human Interface Guidelines), Mikro mimari (micro architecture).
- Platform Desteği: Windows ve GNU/Linux
- Lisans: GPL-3.0

# Ana İşlevsellik
- Bilgisayara bağlı Android cihazları üzerinde uygulama işlemleri yapma.
- Android cihazda kullanıcı tarafından yüklenmiş uygulamaları listeleme ve uygulama bilgilerini detaylı olarak gösterme.
- Kullanıcının listeden seçim yaparak cihazdaki uygulamaları ADB ve AAPT2 araçlarıyla yedekleyip yada doğrudan kaldırabilmesini sağlama.
- Kullanıcının kurduğu uygulamaları otomatik kategorilendirme.
  - Kategoriler: Zararlı, Güvenli, Bilinmeyen.
- Zararlı ve güvenli uygulamaları otomatik tespit etme.
  - Zararlı uygulamaları varsayılan olarak seçili listeleme.
- Seçili uygulamaları yedekleyip yada doğrudan kaldırma.
- Uygulama isimleri, zararlı ve güvenli eşleme listeleri kullanıcının veri dizinindeki yerel metin dosyalarından okunur (`AppNames`, `MaliciousApps`, `SafeApps`).
  - Windows: `%appdata%/Local/Muha/AppManager/`
  - GNU/Linux: `~/.local/share/Muha/AppManager/`
- Toplu uygulama kurulumu: Yedeklenen APK'ları seçerek birden fazla uygulamayı aynı anda cihaza kurma. Kurulum sırası belirleme.
- Uygulama dondurma: Uygulamaları kaldırmadan devre dışı bırakma (`pm disable-user`) ve yeniden etkinleştirme.
- Uygulama izinleri yönetimi: Her uygulamanın izinlerini görüntüleme ve toplu izin verme/kaldırma.
- Uygulama verileri yönetimi: Uygulama önbelleğini temizleme (`pm clear`) ve uygulama verilerini yedekleme/geri yükleme.
- Çoklu cihaz yönetimi: Aynı anda birden fazla cihazı yönetme ve cihazlar arası uygulama kopyalama.

# Arayüz Detayları

## Ana Pencere `Adw.ApplicationWindow`
- Boyutu: yükseklik 800 genişlik 700.
- Başlık: "Android Uygulama Yöneticis".
- Ana yapı `Adw.ToolbarView`:
  - Pencere içeriği `Adw.ToolbarView` ile yapılandırılacak.

## Başlık Çubuğu `Adw.HeaderBar`
- [Sol] Yenile Düğmesi `Gtk.Button`: Simge `view-refresh-symbolic`, ipucu `Listeyi Yenile (Ctrl+R)`.
- [Sol] Seçim Menüsü `Adw.SplitButton`: Simge `edit-select-all-symbolic`, ipucu "Sadece bu sekmedeki uygulamaları seçer/temizler (Ctrl+A)". Menü: "Tümünü Seç/temizle (Ctrl+Shift+A)".
- [Orta] Cihaz Seçimi `Gtk.DropDown`: Başlık widget'ı içinde, etiket "Cihaz:" ve cihaz listesi (Ctrl+Space).
- [Sağ] Arama Düğmesi `Gtk.ToggleButton`: Simge `edit-find-symbolic`, ipucu "Ara (Ctrl+F)". Uygulama listesi görünürken tıklanabilir olur.
- [Sağ] Kaldır Düğmesi `Gtk.Button`: Simge `edit-delete-symbolic`, ipucu "Seçili Uygulamaları Kaldır (Delete)". Varsayılan pasif.
- [Sağ] Menü Düğmesi `Gtk.MenuButton`: Simge `open-menu-symbolic`, ipucu "Ayarlar".
- Menü İçeriği:
  - "Yedekle": Seçili uygulamaların APK'larını bilgisayara yedekler.
  - "İptal" (Escape): Mevcut işlemi iptal eder.
  - "Ayarlar": Ayarlar penceresini açar.
  - "Hakkında": Hakkında penceresini açar.
  - "Çıkış" (Ctrl+Q): Programı kapatır.

## İçerik Alanı
- Kategori Seçici `Gtk.StackSwitcher`: Başlığın altında; "Bilinmeyen", "Zararlı", "Güvenilen", "Tümü" sekmelerini kontrol eder.
- Sıralama Seçici `Gtk.DropDown`: Kategori seçicinin altında; uygulama listesini sıralama seçenekleri.
  - "Ad (A-Z)"
  - "Ad (Z-A)"
  - "Boyut (Küçük-Büyük)"
  - "Boyut (Büyük-Küçük)"
  - "Kurulum Tarihi (Yeni-Eski)"
  - "Kurulum Tarihi (Eski-Yeni)"
  - Görünürlük: Ayarlar → Görünüm → "Sıralama Seçeneklerini Göster" aktif olduğunda görünür.
- Arama alanı `Gtk.SearchEntry`: Arama düğmesi aktifken görünür.
  - Arama filtreleme düğmeleri: Arama alanının altında yatay düzen.
    - "Uygulama Adı" `Gtk.ToggleButton`
    - "Paket Kimliği" `Gtk.ToggleButton`
    - "Boyut" `Gtk.ToggleButton`
    - "Tarih" `Gtk.ToggleButton`
    - "Regex" `Gtk.ToggleButton`: Aktif olduğunda arama alanı düzenli ifade desteği sağlar.
    - Görünürlük: Ayarlar → Görünüm → "Gelişmiş Arama Seçeneklerini Göster" aktif olduğunda görünür.
- Bilgi sayfası `Adw.StatusPage`: Cihaz bağlı olmadığında yada cihaz ile alakalı yapılması gereken bir işlem olduğunda gösterilir.
  - Başlık "Cihaz Bulunamadı". Simge `phone-symbolic`. Mesaj "USB hata ayıklamayı etkinleştir ve cihazı kablo ile bağla.\n\n<b>Ayarlar</b> → <b>Telefon hakkında</b> → <b>Derleme numarası</b>\n<i>5 defa dokun</i>.\n\nCihaz otomatik olarak tanınmazsa <b>Yenile</b> düğmesine tıklayarak yada <b>Ctrl+R</b> tuşlarına basarak cihazın algılanmasını sağla."
  - Başlık: "ADB Erişim İznini". Simge `network-transmit-receive-symbolic`. Mesaj "Telefon ekranında beliren ADB erişim izni isteği mesajını onayla"
  - Başlık: "Uygulama Bulunamadı". Simge `emblem-readonly-symbolic`. Mesaj "Cihazda kullanıcı tarafından kurulan uygulama bulunamadı"
  - Başlık: "ADB Hatası". Simge `dialog-error-symbolic`. Mesaj "Cihazlar listelenemedi.\n\nAndroid cihazda ADB hata ayıklama etkin ve bilgisayarda ADB programı kurulu olmalı.\n\nADB hatası: {burada adb'nin verdiği hata mesajı gösterilecek}"
  - Bildirim katmanı `Adw.ToastOverlay`: Kullanıcı bildirimlerini göstermek için içerik bu katman içine alınır.
  - Uygulama yığını `Gtk.Stack`: "Bilinmeyen", "Zararlı", "Güvenilen", "Tümü" sayfalarını içerir.
  - Her sayfa bir `Gtk.StackPage` içerir:
    - Liste sayfası `Gtk.ScrolledWindow → Gtk.ListBox`: Uygulamaları listeler.
    - Durum sayfası `Adw.StatusPage`: Liste boşken veya özel durumlarda gösterilir.
    - Yükleniyor sayfası `Adw.Spinner`: İşlem sürerken gösterilir.

## Durum Çubuğu (Bottom Bar)
- Pencerenin en altında yatay kutu `Gtk.Box`.
- Solda etiketler: "Seçili: XXX", "Zararlı: XXX", "Güvenli: XXX", "Toplam: XXX".
- Sağda geliştirici e-posta bağlantısı `Gtk.LinkButton`: "Muha Aliss" mailto adresi <muhaaliss@pm.me>.

## Cihaz Seçimi `Gtk.DropDown`
- Birden fazla ADB cihazı bağlıysa, cihaz seçimi için bir açılır liste `Gtk.DropDown` göster.
- Cihaz listesinde marka-model bilgileri yer alsın.
- Cihaz takılma/çıkarılma durumlarında liste otomatik olarak güncellensin.

## Uygulama Listesi `Gtk.ListBox`
- Kullanıcı tarafından kurulmuş uygulamaları listeler `Gtk.ListBox`.
- Liste satırı `Adw.ActionRow`, satır aralarına ince çizgi `Gtk.Separator` yer alsın. Soldan sağa sırayla:
  - Seçme kutusu `Gtk.CheckButton`
  - Kategori değiştirme düğmesi `Gtk.MenuButton`
    - Tıklandığında mevcut kategori dışındaki kategorileri(Zararlı, Güvenli, Bilinmeyen) listeleyen bir popup menü açılır. Kullanıcı bir kategori seçtiğinde uygulama anında o kategoriye taşınır ve ilgili sekmelerde güncellenir.
  - Uygulama simgesi `Gtk.Image`, simge bulunamazsa yer tutucu simge `AndroidHead.svg` gösterilsin.
  - Uygulama adı `Gtk.Label` kalın harflerle.
  - Adın altında uygulama kimliği `Gtk.Label` italik harflerle.
  - Satırın sonunda uygulama detaylarını açmak için bir bilgi düğmesi `Gtk.Button`.
    - Bilgi düğmesi `Gtk.Button`, tıklandığında ilgili uygulamanın bilgilerini gösteren `Adw.Dialog` açılsın.
    - Dialog'un üst kısmında uygulama simgesi, altında uygulama adı, uygulama kimliği, uygulama boyutu, uygulama versiyonu, uygulama yayıncısı, uygulama tarih bilgileri gösterilsin.
    - Dialog'un alt kısmında uygulama kategorisi simgesiyle beraber gösterilsin.
    - Kategori düğmesi aktif ve işevsel olsun, tıklandığında kategori değiştirme popup penceresi açılsın.
    - Dialog'un alt kısmında yedekleme yapma ve doğrudan kaldırma düğmeleri olsun.
    - Dialog dışı bir yere tıklandığında dialog kapansın (modal davranış).
- Boş liste durumu: Eğer bir kategoride hiç uygulama yoksa, `Adw.StatusPage` ile uygun mesaj gösterilsin.

## Simge Yönetimi `Gtk.Image`
- Uygulama simgeleri öncelikle yerel simge dizininde aransın, yoksa adb üzerinden çekilen apk içinden ayıklansın.
- Simge dosyaları "UygulamaKimliği.png" formatında saklanacak.
- Program Simgeleri:
  - Uygulamanın kendi simgeleri `%programfiles%/Muha/AppManager/icons/` klasöründe bulunur.
- APK Simgeleri (Önbellek):
  - Çekilen APK simgeleri kullanıcı veri dizininde saklanır:
    - Windows: `%appdata%/Local/Muha/AppManager/cache/icons`
    - GNU/Linux: `~/.local/share/Muha/AppManager/cache/icons`
- Simge Çıkarma İşlemi (Teknik):
  1. `adb shell pm path <paket_adı>` komutu ile APK yolu bulunur.
  2. `adb pull <uzak_yol> <yerel_gecici_yol>` ile APK geçici bir dizine çekilir.
  3. `aapt2 dump badging <yerel_apk>` komutu ile paket bilgileri okunur ve simge yolu `application-icon` ayrıştırılır.
  4. Simge dosyası APK içinden (zip/unzip kütüphanesi ile) çıkarılır ve önbellek dizinine `paket_adı.png` olarak kaydedilir.
  5. Geçici APK dosyası silinir.
- Simgesi çekilemeyen uygulamalar için yer tutucu simge `android_placeholder.png` kullan.

## Düğmeler
- Yenile `Gtk.Button`: ADB cihaz listesini ve uygulama listesini yeniler.
- Seç/Bırak `Adw.SplitButton`: Sadece görüntülenen kategorideki uygulamaları seçer veya seçimleri kaldırır.
  - Tümünü Seç/Seçimi Kaldır: Tüm kategorilerdeki uygulamaları seçer veya tüm seçimleri kaldırır.
- İptal `Gtk.Button`: Mevcut işlemi iptal eder.
- Kaldır `Gtk.Button`: Seçilen uygulamaları cihazdan kaldırır.
- *Menü `Gtk.MenuButton`:* Menü popup seçimi penceresini açar.
  - *Ayarlar `Gtk.Button`:* Ayarlar penceresini açar.
  - *Simgeleri Al `Gtk.Button`:* Uygulama simgelerini çeker/günceller.
  - *Çıkış `Gtk.Button`:* Programı kapatır.

## Toplu İşlem Şablonları
- Sık kullanılan işlem setlerini kaydetme ve tek tıkla uygulama.
- Örnek şablonlar: "Tüm sosyal medya uygulamalarını kaldır", "Oyun uygulamalarını dondur" vb.

## Ayarlar `Adw.PreferencesDialog`
- Arama özelliği aktif olacak.
- İçerik genişliği 600.
- İçerik yüksekliği 500.

### Genel `Adw.PreferencesPage`
- Simgesi `preferences-system-symbolic`.

#### Görünüm `Adw.PreferencesGroup`
- Arayüz görünüm ayarları
- Tema `Adw.ComboRow`: Uygulamanın arayüz teması
  - Koyu
  - Açık
  - Sistem
- Arayüz Ölçekleme `Adw.ComboRow`: Tüm arayüz öğelerinin ölçekleme oranı
  - "1x (Normal)",
  - "1.25x",
  - "1.5x",
  - "1.75x",
  - "2x",
  - "2.5x",
  - "3x",
- Simgeleri Göster `Adw.SwitchRow`: Uygulama listesinde simgeleri göster
  - Göster (default)
  - Gösterme
- Gelişmiş Arama Seçeneklerini Göster `Adw.SwitchRow`: Arama alanı altında filtreleme düğmelerini göster
  - Kapalı (default)
  - Açık
- Sıralama Seçeneklerini Göster `Adw.SwitchRow`: Kategori seçicinin yanında sıralama dropdown göster
  - Kapalı (default)
  - Açık

#### Uygulama Davranışı `Adw.PreferencesGroup`
- Uygulamanın genel davranış ayarları
- Varsayılan Başlangıç Sekmesi `Adw.ComboRow`: Uygulama başladığında gösterilecek sekme
  - Tümü
  - Güvenilen
  - Zararlı (default)
  - Bilinmeyen
- Otomatik Yenileme `Adw.SwitchRow`: Cihaz bağlandığında otomatik olarak uygulama listesini yenile
  - Açık (default)
  - Kapalı
- Kaldırma Onayı `Adw.SwitchRow`: Uygulamaları kaldırmadan önce onay iste
  - Açık (default)
  - Kapalı
- Sekme Geçişi `Adw.SwitchRow`: Liste yenilendiğinde zararlı uygulama varsa otomatik olarak Zararlı sekmesine geç
  - Açık (default)
  - Kapalı
- Hoş Geldin Turu `Adw.SwitchRow`: Uygulama başladığında hoş geldin penceresini göster
  - Açık (default)
  - Kapalı

#### Görünüm ve Özelleştirme `Adw.PreferencesGroup`
- Arayüz özelleştirme seçenekleri
- Liste Görünümü `Adw.ComboRow`: Uygulama listesi görünüm modu
  - "Liste" (default)
  - "Grid"
  - "Kompakt"
- Favoriler Göster `Adw.SwitchRow`: Favori uygulamaları üstte göster
  - Kapalı (default)
  - Açık

### Araçlar `Adw.PreferencesPage`
- Simgesi `applications-utilities-symbolic`

#### Android Araçları `Adw.PreferencesGroup`
- Android geliştirme araçlarının dizin yolları
- ADB Dizin Yolu `Adw.ActionRow`: Android Debug Bridge aracının bulunduğu dizin.
  - Varsayılan olarak sistem yolunda(environment path) adb programını arasın.
  - Programı bulamazsa Windows için
    - `%programfiles%/Muha/AppManager/tools/platform-tools/adb.exe` dizininde aransın.
  - Programı bulamazsa GNU Linux için
    - `~/.local/share/Muha/AppManager/tools/platform-tools/adb` dizininde aransın.
  - Suffix: `Gtk.Box` içinde `Gtk.Entry` (yol) ve `Gtk.Button` (Gözat).
- AAPT2 Dizin Yolu `Adw.ActionRow`: Android Asset Packaging Tool 2 aracının bulunduğu dizin.
  - Varsayılan olarak sistem yolunda(environment path) aapt2 programını arasın.
  - Programı bulamazsa Windows için
    - `%programfiles%/Muha/AppManager/tools/build-tools/{VERSIONNUMBER}/aapt2.exe` dizininde aransın.
  - Programı bulamazsa GNU Linux için
    - `~/.local/share/Muha/AppManager/tools/build-tools/{VERSIONNUMBER}/aapt2` dizininde aransın.
  - Suffix: `Gtk.Box` içinde `Gtk.Entry` (yol) ve `Gtk.Button` (Gözat).

#### Araç Durumu `Adw.PreferencesGroup`
- Android geliştirme araçlarının durumu
- ADB Durumu `Adw.ActionRow`: Android Debug Bridge aracının durumu.
- Durum `Gtk.Label`: Varsayılan olarak "Kontrol ediliyor..." yazacak.
  - Uygun: ADB aracı sistem yolunda bulundu ve çalışır durumda.
  - Yerel: Program dizininde bulundu ve çalışır durumda.
  - Yok: ADB aracı bulunamadı veya çalışır durumda değil.
- Test Et `Gtk.Button`: Android Debug Bridge aracının durumunu kontrol eder.
- AAPT2 Durumu `Adw.ActionRow`: Android Asset Packaging Tool 2 aracının durumu.
- Durum `Gtk.Label`: Varsayılan olarak "Kontrol ediliyor..." yazacak.
  - Uygun: AAPT2 aracı sistem yolunda bulundu ve çalışır durumda.
  - Yerel: Program dizininde bulundu ve çalışır durumda.
  - Yok: AAPT2 aracı bulunamadı veya çalışır durumda değil.
- Test Et `Gtk.Button`: Android Asset Packaging Tool 2 aracının durumunu kontrol eder.

#### Analiz ve Raporlama `Adw.PreferencesGroup`
Uygulama analiz özellikleri

- Depolama Analizi Göster `Adw.SwitchRow`: Uygulamaların kapladığı alanı grafikle göster
  - Kapalı (default)
  - Açık
- Kurulum Geçmişi Kaydet `Adw.SwitchRow`: Uygulama kurulum ve güncelleme tarihlerini kaydet
  - Kapalı (default)
  - Açık
- İstatistik Toplama `Adw.SwitchRow`: Kullanım istatistiklerini topla (kaldırılan uygulamalar, temizlenen alan vb.)
  - Kapalı (default)
  - Açık

#### Güvenlik ve Gizlilik `Adw.PreferencesGroup`
- Güvenlik analiz özellikleri
- İzin Analizi `Adw.SwitchRow`: Tehlikeli izinlere sahip uygulamaları vurgula
  - Kapalı (default)
  - Açık
- Ağ Aktivitesi İzleme `Adw.SwitchRow`: İnternet kullanan uygulamaları göster
  - Kapalı (default)
  - Açık
- Boot Uygulamaları Göster `Adw.SwitchRow`: Otomatik başlayan uygulamaları listele
  - Kapalı (default)
  - Açık

### Gelişmiş `Adw.PreferencesPage`
- Simgesi `applications-utilities-symbolic`.

#### Yedekleme `Adw.PreferencesGroup`
- Yedekleme ve veri yönetimi ayarları
- APK Yedekleme Konumu `Adw.ActionRow`: APK dosyalarının kaydedileceği dizin
  - Suffix: `Gtk.Box` içinde `Gtk.Label` (seçili yol) ve `Gtk.Button` (Değiştir).
  - Varsayılan: Windows `%userprofile%/Documents/Muha/AppManager/Backups/{model-GG-AA-YYYY}/` GNU/Linux `~/Documents/Muha/AppManager/Backups/{model-GG-AA-YYYY}/`
  - Dosya adı: `{uygulamaadı}.apk`
- Veri Klasörünü Aç `Adw.ActionRow`: Uygulama verilerinin bulunduğu klasörü aç
  - Suffix: `Gtk.Button` (Klasörü Aç). Simge: `folder-open-symbolic`

#### Loglama `Adw.PreferencesGroup`
- İşlem kayıtları ve hata ayıklama
- Loglama Etkin `Adw.SwitchRow`: Uygulama işlemlerinin log dosyasına kaydedilmesini etkinleştir
  - Kapalı (default)
  - Açık
- Log Seviyesi `Adw.ComboRow`: Kaydedilecek log detay seviyesi
  - "Hata" (default)
  - "Uyarı"
  - "Bilgi"
  - "Detaylı"
- Log Klasörünü Aç `Adw.ActionRow`: Log dosyalarının bulunduğu klasörü aç
  - Suffix: `Gtk.Button` (Klasörü Aç). Simge: `folder-open-symbolic`
  - Log dosyaları konumu:
    - Windows: `%appdata%/Local/Muha/AppManager/logs`
    - GNU/Linux: `~/.local/share/Muha/AppManager/logs`
- Logları Temizle `Adw.ActionRow`: Tüm log dosyalarını sil
  - Suffix: `Gtk.Button` (Temizle). Style: `destructive-action`

#### Performans `Adw.PreferencesGroup`
- Uygulama performans ayarları
- Önbellek Boyutu `Adw.SpinRow`: Uygulama simgeleri için önbellek boyutu (MB)
  - Adjustment: Uygulama simgeleri için önbellek boyutunu ayarlar.
    - lower: 10;
    - upper: 1000;
    - step-increment: 10;
    - value: 100;
- Önbelleği Temizle `Adw.ActionRow`: Tüm önbelleği temizle
  - Temizle `Gtk.Button`: Tüm önbelleği temizle

#### Sıfırlama `Adw.PreferencesGroup`
- Uygulama ayarlarını sıfırlama seçenekleri
- Ayarları Sıfırla `Adw.ActionRow`: Tüm ayarları varsayılan değerlere döndür
  - Sıfırla `Gtk.Button`: styles ["destructive-action"]

## Hoş Geldin Turu (Welcome Tour)
- `Adw.Dialog` içinde `Gtk.Overlay` ve `Adw.Carousel` kullanılır.
- Sayfalar:
  1. "Cihazı Kabloyla Bağla" `computer-symbolic`
  2. "Uygulamaları Seç" `edit-select-all-symbolic`
  3. "Kaldır" `edit-delete-symbolic`
  4. "Klavye Kısayolları" `preferences-desktop-keyboard-shortcuts-symbolic`
  5. "Temel ayarları yap" `preferences-other-symbolic` - "Ayarlar" düğmesi içerir.
- Gezinme Düğmeleri: "Atla", "Önceki", "Sonraki", "Bitir".

## Hakkında penceresi `Adw.AboutDialog`
- Uygulama Adı: "Android Uygulama Temizleyici"
- Geliştirici: "Muha Aliss"
- Sürüm: "0.1.0"
- Lisans: GPL-3.0
- Uygulama Simgesi: "com.muha.AppManager"
- Telif Hakkı: "© 2024 Muha Aliss"
- Web Sitesi: https://gitlab.com/muhaaliss
- Sorun Bildirimi: https://gitlab.com/muhaaliss
- Destek: https://gitlab.com/muhaaliss
- Sürüm Notları: HTML formatında sürüm notları (0.1.0).
- Yorumlar: Uygulama açıklaması.

# Teknik Detaylar

## Dosya Yapısı
- `src/main.c`: Ana giriş noktası, programın temel fonksiyonlarını içerir.
- `src/gui.c`: Arayüzle ilgili fonksiyonlar, arayüzler oluşturulur ve yerleşim planları burada yapılır.
- `src/utils.c`: Yardımcı fonksiyonlar (cihaz listeleme, yedekleme, vb.).
- `src/adb.c`: ADB işlemlerini yöneten fonksiyonlar.
- `src/error.c`: Hata kontrolü fonksiyonları.
- `src/prefs.c`: Ayar sayfası fonksiyonları.
- `src/welcome.c`: Hoş geldin turu fonksiyonları.
- `src/about.c`: Hakkında fonksiyonları.
- `include/`: Başlık dosyaları.
- `tools/`: ADB ve AAPT2 dosyalarının bulunacağı dizin. İçerisinde `platform-tools`, `build-tools`, `cmdline-tools` gibi alt dizinler barındırır.
- `data/`: Uygulama verilerini barındıran dizin.
  - `packaging/`: Exe, DEB, RPM, AppImage dağıtım dosyalarının nasıl oluşturulacağı bilgilerinin tutulduğu dizin.
    - `EXE/`: Program için bir NSS kurucu(nullsoft installer) nasıl oluşturulacağı betiğinin bulunduğu dizin.
    - `DEB/`: Program için bir DEB paketinin nasıl oluşturulacağı betiğinin bulunduğu dizin.
    - `RPM/`: Program için bir RPM paketinin nasıl oluşturulacağı betiğinin bulunduğu dizin.
    - `APPIMAGE/`: Program için bir AppImage paketinin nasıl oluşturulacağı betiğinin bulunduğu dizin.

## ADB entegrasyonu
- Program, öncelikle sistem ortam yollarında (environment path) ADB'nin varlığını kontrol etmeli.
- Sistemde ADB bulunamazsa, `%programfiles%/Muha/AppManager/tools/platform-tools` dizinindeki ADB'yi kullanmalı.
- Cihaz bağlantı durumu kontrol edilmeli ve kullanıcıya bilgi verilmeli (örneğin, "Cihaz bağlı değil", "ADB erişim izni gerekli").
- Varsayılan olarak yalnızca kullanıcı tarafından yüklenen uygulamalar (`-3` bayrağı) listelenmeli.

## Asenkron işlemler
- Gtk4'ün GTask veya sinyal mekanizmaları kullanılarak ADB komutları gibi uzun süren işlemler ayrı thread'lerde çalıştırılmalı.

## Hata Yönetimi
- ADB komutları başarısız olursa duruma uygun şekilde kullanıcıya hata mesajları `Adw.StatusPage`, `Adw.AlertDialog` veya `Adw.Toast` kullanılarak gösterilmeli.

## Kullanıcı Onayları
- Kaldırma işlemi gibi kritik işlemler öncesinde kullanıcıdan `Adw.AlertDialog` ile onay istenmeli (örneğin, "Seçilen uygulamalar kaldırılsın mı?").

## İşlem Geri Bildirimi
- Kaldırma işlemi sırasında uygulama listesi yerine "İşlem devam ediyor" göstergesi `Adw.StatusPage` içinde `Adw.Spinner` ve tek tek işlenen uygulama adı `Adw.Label` kaldırılıyor şeklinde gösterilmeli. İşlem tamamlandığında liste tekrar yüklenm eli.
- Yedekleme işlemi sırasında:
  - Yedekleme başladığında uygulama listesi yerine "İşlem devam ediyor" göstergesi `Adw.StatusPage` içinde `Adw.Spinner` ve işlenen uygulama bilgisi gösterilmeli.
  - İşlem tamamlandığında `Adw.Toast` bildirimi gösterilmeli:
    - Mesaj: "X uygulama başarıyla yedeklendi"
    - Bildirimde "Klasörü Aç" düğmesi olmalı, tıklandığında yedekleme dizini açılmalı.
    - Bildirim varsayılan süre (5 saniye) kadar görünür olmalı.

# Derleme Modları

## Geliştirici Derlemesi (Debug Mode)
- Koddaki ek kontrol adımları ve hata ayıklama mesajları terminale yazdırılsın.
- Her yapılan işlemden sonra işlem mesajı terminale yazdırılsın.

## Yayınlanabilir Derleme (Release Mode)
- Geliştirici mesajlarını içermemeli.
- Son kullanıcı için optimize edilmeli.

## Dağıtım Derlemesi (Distribution Mode)
- Yayınlanabilir derlemeyi gerçekleştirsin.
- Ardından `tools/` içindeki `platform-tools` ve `build-tools` dizinlerini toplasın.
- Bir kurucu (installer) paketi hazırlasın (Windows için NSIS, Linux için DEB-RPM-AppImage).
- Windows kurulum dosyasında kullanıcıya yardımcı programları(ADB, AAPT2) kurmak isteyip istemediğini seçeneği sunulsun.

# Ek Özellikler ve İstenenler
- Dil Desteği: Şuanda proje sadece Türkçe.
- Etiketleme Sistemi: Uygulamalara özel etiketler ekleme ve etiket bazlı filtreleme.
