# Android App Manager (MuhaAppManager)

<div align="center">

![Android App Manager](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-blue)
![License](https://img.shields.io/badge/License-GPL--3.0-green)
![Language](https://img.shields.io/badge/Language-C-orange)
![GTK](https://img.shields.io/badge/GTK-4%20%7C%20Libadwaita-purple)
![AI](https://img.shields.io/badge/Vibe%20Coded-90%25-black)

**Easily manage applications on your Android devices connected to your computer**

[Features](#-features) • [Installation](#-installation) • [Usage](#-usage) • [Development](#-development) • [License](#-license) • [Türkçe](README-TR.md)

</div>

---

## 📋 Table of Contents

- [About](#-about)
- [Features](#-features)
- [Screenshots](#-screenshots)
- [Installation](#-installation)
- [Usage](#-usage)
- [Keyboard Shortcuts](#-keyboard-shortcuts)
- [Development](#-development)
- [Contributing](#-contributing)
- [License](#-license)
- [Contact](#-contact)

---

## 📖 About

**Android App Manager** is a modern desktop application that allows you to manage applications on Android devices connected to your computer via USB. Using ADB (Android Debug Bridge) and AAPT2 tools, you can easily list, backup, uninstall, and categorize applications.

### Why Android App Manager?

- 🎯 **Smart Categorization**: Automatically categorizes your apps into Malicious, Safe, and Unknown categories
- 🛡️ **Security Focused**: Detects malicious apps and displays them as selected by default
- 🎨 **Modern Interface**: Sleek and user-friendly interface with GNOME Adwaita design language
- ⚡ **Fast and Efficient**: Written in C, lightweight and performant
- 🔧 **Advanced Features**: Batch operations, app freezing, permission management, and more

---

## ✨ Features

### Core Features

- ✅ **App Listing**: Lists user apps on your device with detailed information
- 🗂️ **Smart Categorization**: Automatically classifies apps as Malicious, Safe, and Unknown
- 💾 **APK Backup**: Backs up APK files of selected apps to your computer
- 🗑️ **Batch Uninstall**: Uninstalls multiple apps at once
- 🔍 **Advanced Search**: Filter by app name, package ID, size, and date
- 📱 **Multi-Device Support**: Manage multiple Android devices simultaneously

### Advanced Features

- 🔄 **Batch Installation**: Select backed up APKs to install multiple apps at once
- ❄️ **App Freezing**: Disable apps without uninstalling them (`pm disable-user`)
- 🔐 **Permission Management**: View app permissions and grant/revoke permissions in bulk
- 🧹 **Data Management**: Clear app cache and backup/restore app data
- 🔄 **Cross-Device Copy**: Copy apps between different devices
- 📊 **Operation Templates**: Save frequently used operation sets and apply with one click

### User Interface

- 🎨 **Modern Design**: Interface compliant with GNOME design standards using Libadwaita and GTK4
- 🌓 **Theme Support**: Light, Dark, and System theme options
- 📐 **Scaling**: Interface scaling support from 1x to 3x
- 🔍 **Smart Search**: Advanced search filtering with regex support
- 📋 **Flexible Views**: List, Grid, and Compact view modes
- ⌨️ **Keyboard Shortcuts**: Keyboard shortcuts for all important operations

---

## 🖼️ Screenshots

> *Screenshots will be added soon*

---

## 🚀 Installation

### System Requirements

- **Operating System**: Windows 10/11 or GNU/Linux
- **Dependencies**:
  - GTK4
  - Libadwaita
  - ADB (Android Debug Bridge)
  - AAPT2 (Android Asset Packaging Tool 2)

### Windows

1. Download the latest version from the [Releases](https://gitlab.com/muhaaliss/appmanager/releases) page
2. Run the `.exe` installer
3. Check the option to install ADB and AAPT2 tools during installation (recommended)
4. Launch the application after installation is complete

### Linux

#### DEB-based Distributions (Ubuntu, Debian, Linux Mint)

```bash
# Download the DEB package
wget https://gitlab.com/muhaaliss/appmanager/releases/download/v0.1.0/appmanager_0.1.0_amd64.deb

# Install the package
sudo dpkg -i appmanager_0.1.0_amd64.deb

# Install dependencies
sudo apt-get install -f
```

#### RPM-based Distributions (Fedora, RHEL, CentOS)

```bash
# Download the RPM package
wget https://gitlab.com/muhaaliss/appmanager/releases/download/v0.1.0/appmanager-0.1.0-1.x86_64.rpm

# Install the package
sudo rpm -i appmanager-0.1.0-1.x86_64.rpm
```

#### AppImage

```bash
# Download the AppImage file
wget https://gitlab.com/muhaaliss/appmanager/releases/download/v0.1.0/AppManager-0.1.0-x86_64.AppImage

# Make it executable
chmod +x AppManager-0.1.0-x86_64.AppImage

# Run it
./AppManager-0.1.0-x86_64.AppImage
```

### ADB Installation

If ADB is not installed on your system:

**Windows:**
```powershell
# With winget
winget install Google.PlatformTools

# Or manually download Android SDK Platform Tools
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

## 📱 Usage

### Initial Setup

1. **Enable USB Debugging**:
   - On your Android device: `Settings` → `About Phone` → `Build Number` (tap 7 times)
   - `Settings` → `Developer Options` → `USB Debugging` (Enable)

2. **Connect Device**:
   - Connect your Android device to your computer with a USB cable
   - Approve the "USB Debugging" permission that appears on your phone screen

3. **Launch Application**:
   - Open Android App Manager
   - Your device will be detected automatically
   - On first use, the "Welcome Tour" will introduce you to the basic features

### Basic Operations

#### Listing Apps

- When the app opens, all user apps on your device are automatically listed
- Categories: **Malicious**, **Safe**, **Unknown**, **All**
- Malicious apps are selected by default

#### Uninstalling Apps

1. Select the apps you want to uninstall (or use default selections)
2. Click the **Uninstall** button in the header bar (or press `Delete` key)
3. Confirm the operation in the confirmation window
4. The list will be automatically refreshed when the operation is complete

#### APK Backup

1. Select the apps you want to backup
2. Click the menu button → **Backup**
3. A notification will be displayed when the backup is complete
4. Click the **Open Folder** button to open the backup directory

#### App Search

1. Click the **Search** button in the header bar (or `Ctrl+F`)
2. Enter your search term in the search box
3. Use advanced search options for filtering:
   - App Name
   - Package ID
   - Size
   - Date
   - Regex (Regular Expression)

### Advanced Operations

#### Batch Installation

1. Menu → **Install**
2. Select backed up APK files
3. Set installation order (optional)
4. Click the **Install** button

#### App Freezing

1. Select the app you want to freeze
2. Open app details (double click)
3. Click the **Freeze** button
4. Click the **Enable** button to reactivate

#### Permission Management

1. Open app details
2. Go to the **Permissions** tab
3. View and manage permissions
4. Use the relevant buttons for bulk grant/revoke

---

## ⌨️ Keyboard Shortcuts

| Shortcut | Function |
|----------|----------|
| `Ctrl+R` | Refresh List |
| `Ctrl+A` | Select/Clear All in Current Tab |
| `Ctrl+Shift+A` | Select/Clear All in All Tabs |
| `Ctrl+F` | Toggle Search |
| `Ctrl+Space` | Open Device Selection Menu |
| `Delete` | Uninstall Selected Apps |
| `Escape` | Cancel Current Operation |
| `Ctrl+Q` | Quit Application |

---

## 🛠️ Development

### Building from Source

#### Requirements

- GCC or Clang compiler
- Meson build system
- Ninja build tool
- GTK4 development libraries
- Libadwaita development libraries

#### Build Steps

```bash
# Clone the repository
git clone https://gitlab.com/muhaaliss/appmanager.git
cd appmanager

# Create build directory
meson setup build

# Compile
meson compile -C build

# Run
./build/src/appmanager
```

#### Build Modes

**Debug Build**
```bash
meson setup build --buildtype=debug
meson compile -C build
```

**Release Build**
```bash
meson setup build --buildtype=release
meson compile -C build
```

**Distribution Build**
```bash
meson setup build --buildtype=release -Ddistribution=true
meson compile -C build
meson dist -C build
```

### Project Structure

```
appmanager/
├── src/
│   ├── main.c          # Main entry point
│   ├── gui.c           # GUI functions
│   ├── utils.c         # Utility functions
│   ├── adb.c           # ADB operations
│   ├── error.c         # Error handling
│   ├── prefs.c         # Preferences
│   ├── welcome.c       # Welcome tour
│   └── about.c         # About window
├── include/            # Header files
├── data/               # Application data
│   ├── icons/          # Icons
│   └── packaging/      # Packaging scripts
├── tools/              # ADB and AAPT2 tools
└── meson.build         # Build configuration
```

### Code Standards

- **Language Standard**: GNU C
- **Code Style**: GNU coding style
- **Architecture**: Micro architecture
- **UI Standards**: GNOME HIG (Human Interface Guidelines)

---

## 🤝 Contributing

We welcome your contributions! To contribute to the project:

1. Fork the project
2. Create a new branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: Add amazing feature'`)
4. Push your branch (`git push origin feature/amazing-feature`)
5. Create a Pull Request

### Contribution Guidelines

- Follow code standards (GNU C, GNOME HIG)
- Use [Conventional Commits](https://www.conventionalcommits.org/) for commit messages
- Add documentation for new features

---

## 📄 License

This project is licensed under the [GPL-3.0 License](LICENSE).

```
Android App Manager
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

## 📧 Contact

**Developer**: Muha Aliss
**Email**: [muhaaliss@pm.me](mailto:muhaaliss@pm.me)
**GitLab**: [https://gitlab.com/muhaaliss](https://gitlab.com/muhaaliss)
**Issue Reporting**: [GitLab Issues](https://gitlab.com/muhaaliss/appmanager/issues)

---

## 🙏 Acknowledgments

- [GNOME Project](https://www.gnome.org/) - For GTK4 and Libadwaita
- [Android Open Source Project](https://source.android.com/) - For ADB and AAPT2 tools
- All contributors and users

---

<div align="center">

**⭐ Don't forget to star the project if you like it! ⭐**

[🔝 Back to Top](#android-app-manager-muhaappmanager)

</div>
