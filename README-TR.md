<div align="center">
<h1>Android Uygulama Yöneticisi (MuhaAppManager)</h1>

![Android App Manager](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-blue)
![License](https://img.shields.io/badge/License-GPL--3.0-green)
![Language](https://img.shields.io/badge/Language-C-orange)
![GTK](https://img.shields.io/badge/GTK-4%20%7C%20Libadwaita-purple)
![AI](https://img.shields.io/badge/AI%20ile%20%C3%A7ok%20az%20g%C3%B6zetimle%20%C3%BCretildi-90%25-black)

**Bilgisayarınıza bağlı Android cihazlarınızdaki uygulamaları kolayca yönetin**

[Özellikler](#-özellikler) • [Kurulum](#-kurulum) • [Kullanım](#-kullanım) • [Geliştirme](#-geliştirme) • [Lisans](#-lisans) • [English](README.md)

</div>

---

## 📋 İçindekiler

- [Hakkında](#-hakkında)
- [Özellikler](#-özellikler)
- [Ekran Görüntüleri](#-ekran-görüntüleri)
- [Kurulum](#-kurulum)
- [Kullanım](#-kullanım)
- [Klavye Kısayolları](#-klavye-kısayolları)
- [Geliştirme](#-geliştirme)
- [Katkıda Bulunma](#-katkıda-bulunma)
- [Lisans](#-lisans)
- [İletişim](#-iletişim)

---

## 📖 Hakkında

**Android Uygulama Yöneticisi**, bilgisayarınıza USB ile bağlı Android cihazlarınızdaki uygulamaları yönetmenizi sağlayan modern bir masaüstü uygulamasıdır. ADB (Android Debug Bridge) ve AAPT2 araçlarını kullanarak uygulamaları listeleme, yedekleme, kaldırma ve kategorilendirme işlemlerini kolayca yapabilirsiniz.

### Neden Android Uygulama Yöneticisi?

- 🎯 **Akıllı Kategorilendirme**: Uygulamalarınızı otomatik olarak Zararlı, Güvenli ve Bilinmeyen kategorilere ayırır
- 🛡️ **Güvenlik Odaklı**: Zararlı uygulamaları tespit eder ve varsayılan olarak seçili halde gösterir
- 🎨 **Modern Arayüz**: GNOME Adwaita tasarım dili ile şık ve kullanıcı dostu arayüz
- ⚡ **Hızlı ve Verimli**: C dili ile yazılmış, hafif ve performanslı
- 🔧 **Gelişmiş Özellikler**: Toplu işlemler, uygulama dondurma, izin yönetimi ve daha fazlası

---

## ✨ Özellikler

### Temel Özellikler

- ✅ **Uygulama Listeleme**: Cihazınızdaki kullanıcı uygulamalarını detaylı bilgilerle listeler
- 🗂️ **Akıllı Kategorilendirme**: Uygulamaları Zararlı, Güvenli ve Bilinmeyen olarak otomatik sınıflandırır
- 💾 **APK Yedekleme**: Seçili uygulamaların APK dosyalarını bilgisayarınıza yedekler
- 🗑️ **Toplu Kaldırma**: Birden fazla uygulamayı aynı anda kaldırır
- 🔍 **Gelişmiş Arama**: Uygulama adı, paket kimliği, boyut ve tarihe göre filtreleme
- 📱 **Çoklu Cihaz Desteği**: Birden fazla Android cihazı aynı anda yönetin

### Gelişmiş Özellikler

- 🔄 **Toplu Kurulum**: Yedeklenen APK'ları seçerek birden fazla uygulamayı aynı anda kurun
- ❄️ **Uygulama Dondurma**: Uygulamaları kaldırmadan devre dışı bırakın (`pm disable-user`)
- 🔐 **İzin Yönetimi**: Uygulama izinlerini görüntüleyin ve toplu izin verme/kaldırma yapın
- 🧹 **Veri Yönetimi**: Uygulama önbelleğini temizleyin ve verileri yedekleyin/geri yükleyin
- 🔄 **Cihazlar Arası Kopyalama**: Uygulamaları farklı cihazlar arasında kopyalayın
- 📊 **İşlem Şablonları**: Sık kullanılan işlem setlerini kaydedin ve tek tıkla uygulayın

### Kullanıcı Arayüzü

- 🎨 **Modern Tasarım**: Libadwaita ve GTK4 ile GNOME tasarım standartlarına uygun arayüz
- 🌓 **Tema Desteği**: Açık, Koyu ve Sistem teması seçenekleri
- 📐 **Ölçeklendirme**: 1x'den 3x'e kadar arayüz ölçeklendirme desteği
- 🔍 **Akıllı Arama**: Regex desteği ile gelişmiş arama filtreleme
- 📋 **Esnek Görünüm**: Liste, Grid ve Kompakt görünüm modları
- ⌨️ **Klavye Kısayolları**: Tüm önemli işlemler için klavye kısayolları

---

## 🖼️ Ekran Görüntüleri

> *Ekran görüntüleri yakında eklenecektir*

---

## 🚀 Kurulum

### Sistem Gereksinimleri

- **İşletim Sistemi**: Windows 10/11 veya GNU/Linux
- **Bağımlılıklar**:
  - GTK4
  - Libadwaita
  - ADB (Android Debug Bridge)
  - AAPT2 (Android Asset Packaging Tool 2)

### Windows

1. En son sürümü [Releases](https://gitlab.com/muhaaliss/appmanager/releases) sayfasından indirin
2. `.exe` kurulum dosyasını çalıştırın
3. Kurulum sırasında ADB ve AAPT2 araçlarını kurma seçeneğini işaretleyin (önerilir)
4. Kurulum tamamlandıktan sonra uygulamayı başlatın

### Linux

#### DEB Tabanlı Dağıtımlar (Ubuntu, Debian, Linux Mint)

```bash
# DEB paketini indirin
wget https://gitlab.com/muhaaliss/appmanager/releases/download/v0.1.0/appmanager_0.1.0_amd64.deb

# Paketi kurun
sudo dpkg -i appmanager_0.1.0_amd64.deb

# Bağımlılıkları yükleyin
sudo apt-get install -f
```

#### RPM Tabanlı Dağıtımlar (Fedora, RHEL, CentOS)

```bash
# RPM paketini indirin
wget https://gitlab.com/muhaaliss/appmanager/releases/download/v0.1.0/appmanager-0.1.0-1.x86_64.rpm

# Paketi kurun
sudo rpm -i appmanager-0.1.0-1.x86_64.rpm
```

#### AppImage

```bash
# AppImage dosyasını indirin
wget https://gitlab.com/muhaaliss/appmanager/releases/download/v0.1.0/AppManager-0.1.0-x86_64.AppImage

# Çalıştırılabilir yapın
chmod +x AppManager-0.1.0-x86_64.AppImage

# Çalıştırın
./AppManager-0.1.0-x86_64.AppImage
```

### ADB Kurulumu

Eğer sisteminizde ADB kurulu değilse:

**Windows:**
```powershell
# winget ile
winget install Google.PlatformTools

# Veya manuel olarak Android SDK Platform Tools indirin
# https://developer.android.com/studio/releases/platform-tools
```

**Linux:**
```bash
# Debian/Ubuntu
sudo apt install adb

# Fedora
sudo dnf install android-tools

# Arch Linux
sudo pacman -S android-tools
```

---

## 📱 Kullanım

### İlk Kurulum

1. **USB Hata Ayıklamayı Etkinleştirin**:
   - Android cihazınızda `Ayarlar` → `Telefon Hakkında` → `Derleme Numarası` (7 kez dokunun)
   - `Ayarlar` → `Geliştirici Seçenekleri` → `USB Hata Ayıklama` (Etkinleştirin)

2. **Cihazı Bağlayın**:
   - Android cihazınızı USB kablosu ile bilgisayara bağlayın
   - Telefon ekranında beliren "USB Hata Ayıklama" iznini onaylayın

3. **Uygulamayı Başlatın**:
   - Android Uygulama Yöneticisi'ni açın
   - Cihazınız otomatik olarak algılanacaktır
   - İlk kullanımda "Hoş Geldin Turu" size temel özellikleri tanıtacaktır

### Temel İşlemler

#### Uygulamaları Listeleme

- Uygulama açıldığında cihazınızdaki tüm kullanıcı uygulamaları otomatik olarak listelenir
- Kategoriler: **Zararlı**, **Güvenli**, **Bilinmeyen**, **Tümü**
- Zararlı uygulamalar varsayılan olarak seçili gelir

#### Uygulama Kaldırma

1. Kaldırmak istediğiniz uygulamaları seçin (veya varsayılan seçimleri kullanın)
2. Başlık çubuğundaki **Kaldır** düğmesine tıklayın (veya `Delete` tuşuna basın)
3. Onay penceresinde işlemi onaylayın
4. İşlem tamamlandığında liste otomatik olarak yenilenecektir

#### APK Yedekleme

1. Yedeklemek istediğiniz uygulamaları seçin
2. Menü düğmesine tıklayın → **Yedekle**
3. Yedekleme işlemi tamamlandığında bildirim gösterilecektir
4. **Klasörü Aç** düğmesine tıklayarak yedekleme dizinini açabilirsiniz

#### Uygulama Arama

1. Başlık çubuğundaki **Arama** düğmesine tıklayın (veya `Ctrl+F`)
2. Arama kutusuna arama teriminizi girin
3. Gelişmiş arama seçeneklerini kullanarak filtreleme yapın:
   - Uygulama Adı
   - Paket Kimliği
   - Boyut
   - Tarih
   - Regex (Düzenli İfade)

### Gelişmiş İşlemler

#### Toplu Kurulum

1. Menü → **Kurulum**
2. Yedeklenen APK dosyalarını seçin
3. Kurulum sırasını belirleyin (isteğe bağlı)
4. **Kur** düğmesine tıklayın

#### Uygulama Dondurma

1. Dondurmak istediğiniz uygulamayı seçin
2. Uygulama detaylarını açın (çift tıklama)
3. **Dondur** düğmesine tıklayın
4. Yeniden etkinleştirmek için **Etkinleştir** düğmesine tıklayın

#### İzin Yönetimi

1. Uygulama detaylarını açın
2. **İzinler** sekmesine gidin
3. İzinleri görüntüleyin ve yönetin
4. Toplu izin verme/kaldırma için ilgili düğmeleri kullanın

---

## ⌨️ Klavye Kısayolları

| Kısayol | İşlev |
|---------|-------|
| `Ctrl+R` | Listeyi Yenile |
| `Ctrl+A` | Mevcut Sekmedeki Tümünü Seç/Temizle |
| `Ctrl+Shift+A` | Tüm Sekmelerdeki Tümünü Seç/Temizle |
| `Ctrl+F` | Arama Aç/Kapat |
| `Ctrl+Space` | Cihaz Seçimi Menüsünü Aç |
| `Delete` | Seçili Uygulamaları Kaldır |
| `Escape` | Mevcut İşlemi İptal Et |
| `Ctrl+Q` | Uygulamadan Çık |

---

## 🛠️ Geliştirme

### Kaynak Koddan Derleme

#### Gereksinimler

- GCC derleyici
- Meson build sistemi
- Ninja build aracı
- GTK4 geliştirme kütüphaneleri
- Libadwaita geliştirme kütüphaneleri

#### Derleme Adımları

```bash
# Depoyu klonlayın
git clone https://gitlab.com/muhaaliss/appmanager.git
cd appmanager

# Build dizinini oluşturun
meson setup build

# Derleyin
meson compile -C build

# Çalıştırın
./build/src/appmanager
```

#### Derleme Modları

**Geliştirici Derlemesi (Debug)**
```bash
meson setup build --buildtype=debug
meson compile -C build
```

**Yayınlanabilir Derleme (Release)**
```bash
meson setup build --buildtype=release
meson compile -C build
```

**Dağıtım Derlemesi (Distribution)**
```bash
meson setup build --buildtype=release -Ddistribution=true
meson compile -C build
meson dist -C build
```

### Proje Yapısı

```
appmanager/
├── src/
│   ├── main.c          # Ana giriş noktası
│   ├── gui.c           # Arayüz fonksiyonları
│   ├── utils.c         # Yardımcı fonksiyonlar
│   ├── adb.c           # ADB işlemleri
│   ├── error.c         # Hata yönetimi
│   ├── prefs.c         # Ayarlar
│   ├── welcome.c       # Hoş geldin turu
│   └── about.c         # Hakkında penceresi
├── include/            # Başlık dosyaları
├── data/               # Uygulama verileri
│   ├── icons/          # Simgeler
│   └── packaging/      # Paketleme betikleri
├── tools/              # ADB ve AAPT2 araçları
└── meson.build         # Build yapılandırması
```

### Kod Standartları

- **Dil Standardı**: GNU C
- **Kod Stili**: GNU yazım şekli
- **Mimari**: Mikro mimari (micro architecture)
- **UI Standartları**: GNOME HIG (Human Interface Guidelines)

---

## 🤝 Katkıda Bulunma

Katkıları memnuniyetle kabul ediyorum! Projeye katkıda bulunmak için:

1. Projeyi çatalla
2. Yeni bir dal oluştur (`git checkout -b feature/amazing-feature`)
3. Değişiklikleri ekle (`git commit -m 'feat: Add amazing feature'`)
4. Dalı gönder (`git push origin feature/amazing-feature`)
5. Birleştirme isteği oluştur

### Katkı Kuralları

- Kod standartlarına uyun (GNU C, GNOME HIG)
- Commit mesajlarında [Conventional Commits](https://www.conventionalcommits.org/) kullanın
- Yeni özellikler için belge ekle

---

## 📄 Lisans

Bu proje [GPL-3.0 Lisansı](LICENSE) altında lisanslanmıştır.

```
Android Uygulama Yöneticisi
Copyright (C) 2024 Muha Aliss

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
```

---

## 📧 İletişim

**Geliştirici**: Muha Aliss
**E-posta**: [muhaaliss@pm.me](mailto:muhaaliss@pm.me)
**GitLab**: [https://gitlab.com/muhaaliss](https://gitlab.com/muhaaliss)
**Sorun Bildirimi**: [GitLab Issues](https://gitlab.com/muhaaliss/appmanager/issues)

---

## 🙏 Teşekkürler

- [GNOME Projesi](https://www.gnome.org/) - GTK4 ve Libadwaita için
- [Android Open Source Project](https://source.android.com/) - ADB ve AAPT2 araçları için
- Tüm katkıda bulunanlara ve kullanıcılara

---

<div align="center">

**⭐ Projeyi beğendiysen yıldız vermeyi unutma! ⭐**

[🔝 Başa Dön](#android-uygulama-yöneticisi-muhaappmanager)

</div>
