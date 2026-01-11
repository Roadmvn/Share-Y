# ShareY 📸

A lightweight, performant screenshot manager for Linux, inspired by ShareX for Windows.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)

## ✨ Features

- **Screenshot Capture**
  - Fullscreen capture
  - Region selection with visual overlay
  - Window capture
  - Active window capture
  
- **In-Memory Storage**
  - Screenshots stored in RAM (no disk writes until you save)
  - Memory limit with automatic eviction of old captures
  - Thumbnail dashboard for quick overview

- **Clipboard Integration**
  - Double-click to copy to clipboard
  - Context menu for quick actions

- **System Tray**
  - Runs in background
  - Quick access via tray menu
  - Global hotkey support

## 🔧 Requirements

### Build Dependencies

```bash
# Debian/Ubuntu
sudo apt install \
    build-essential cmake ninja-build \
    qt6-base-dev qt6-tools-dev \
    libxcb1-dev libxcb-util-dev libxcb-keysyms1-dev \
    libxcb-image0-dev libxcb-shm0-dev libxcb-xfixes0-dev

# Arch Linux
sudo pacman -S \
    base-devel cmake ninja \
    qt6-base qt6-tools \
    libxcb xcb-util xcb-util-keysyms

# Fedora
sudo dnf install \
    cmake ninja-build gcc-c++ \
    qt6-qtbase-devel qt6-qttools-devel \
    libxcb-devel xcb-util-devel xcb-util-keysyms-devel
```

## 🏗️ Building

```bash
# Clone the repository
git clone https://github.com/yourusername/shareY.git
cd shareY

# Create build directory
mkdir build && cd build

# Configure
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
ninja

# Run
./shareY
```

## 🚀 Usage

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `PrintScreen` | Capture fullscreen |
| `Ctrl+PrintScreen` | Capture region |
| `Alt+PrintScreen` | Capture window |
| `Shift+PrintScreen` | Capture active window |
| `Escape` | Hide window / Cancel |

### Dashboard

- **Single click** - Select screenshot
- **Double click** - Copy to clipboard
- **Right click** - Context menu (copy, save, delete, edit)

## 📁 Project Structure

```
shareY/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── main.cpp            # Application entry point
│   ├── core/
│   │   ├── Screenshot.hpp  # Screenshot data structure
│   │   └── ScreenshotBuffer.hpp/cpp  # In-memory storage
│   ├── capture/
│   │   ├── CaptureEngine.hpp/cpp     # Abstract capture interface
│   │   └── X11Capture.hpp/cpp        # XCB implementation
│   ├── hotkeys/
│   │   ├── HotkeyManager.hpp/cpp     # Global hotkey manager
│   │   └── X11Hotkey.hpp/cpp         # X11 key grabbing
│   └── ui/
│       ├── MainWindow.hpp/cpp        # Dashboard window
│       ├── ThumbnailWidget.hpp/cpp   # Screenshot thumbnail
│       └── RegionSelector.hpp/cpp    # Region selection overlay
└── resources/
    ├── shareY.qrc          # Qt resources
    ├── style.qss           # Stylesheet
    └── icons/              # Application icons
```

## 🔮 Roadmap

- [ ] Annotation engine (arrows, rectangles, text, blur)
- [ ] Wayland support via xdg-desktop-portal
- [ ] Multi-monitor support with RANDR
- [ ] Upload to services (Imgur, custom servers)
- [ ] GIF recording
- [ ] Settings dialog
- [ ] AppImage/Flatpak packaging

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

## 🙏 Acknowledgments

- Inspired by [ShareX](https://getsharex.com/) for Windows
- Uses [Catppuccin Mocha](https://github.com/catppuccin/catppuccin) color scheme
- Built with Qt6 and XCB
