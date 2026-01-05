# Yandex Translator Desktop App

A lightweight, high-performance desktop application for Windows (10/11) that provides quick access to Yandex Translate via a floating overlay window.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)

## Features

✨ **Lightweight & Fast**
- Minimal memory footprint (~30-50MB RAM)
- Sub-2 second startup time
- Efficient Qt WebEngine integration

🎯 **Global Hotkey**
- Customizable global hotkey (default: Ctrl+Alt+T)
- Quick toggle of translator window
- Automatic clipboard text insertion

🪟 **Floating Overlay**
- Always-on-top mode toggle
- Adjustable transparency (20-100%)
- Drag-to-move functionality
- Frameless, minimalist design

📋 **Smart Clipboard**
- Automatic text detection
- Direct insertion into translator
- Text length validation

⚙️ **System Integration**
- Minimize to system tray
- Auto-start on Windows login
- Context menu for quick access
- Network status monitoring

🛡️ **Robust Error Handling**
- Offline mode support
- Graceful degradation on network failures
- Automatic retry on load errors

## Screenshots

*Add screenshots here*

## System Requirements

- **OS**: Windows 10 or 11 (64-bit)
- **RAM**: 512MB minimum, 2GB recommended
- **Disk**: 50MB free space
- **Network**: Internet connection for translation

## Installation

### From Release

1. Download the latest release from [Releases](https://github.com/yourusername/YandexTranslator/releases)
2. Extract the ZIP file
3. Run `YandexTranslator.exe`
4. The application will start minimized to tray

### Building from Source

See [BUILD.md](BUILD.md) for detailed build instructions.

## Usage

### Quick Start

1. **Launch the application** - It will start in the system tray
2. **Press Ctrl+Alt+T** (or your custom hotkey) to show the translator
3. **Copy text** to your clipboard before activating the hotkey
4. The text will be **automatically inserted** into the translator
5. **Press the hotkey again** to hide the window

### Tray Icon Menu

Right-click the tray icon to access:

- **Show/Hide Window** - Toggle window visibility
- **Toggle Always on Top** - Keep window above other apps
- **Settings...** - Configure hotkey, transparency, etc.
- **Reload Translator** - Reload the translator page
- **Exit** - Close the application

### Settings

Configure the following options:

| Setting | Description | Default |
|----------|-------------|---------|
| Hotkey | Global hotkey to toggle window | Ctrl+Alt+T |
| Always on Top | Keep window above other windows | Enabled |
| Opacity | Window transparency (20-100%) | 90% |
| Auto Start | Launch on Windows login | Enabled |
| Minimize to Tray | Hide to system tray instead of taskbar | Enabled |

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+Alt+T | Toggle translator window (default) |
| Ctrl+Shift+T | Show translator without clipboard insertion |

## Configuration

Configuration is stored in:

```
%APPDATA%\YandexTranslator\settings.json
```

Example configuration:

```json
{
  "hotkey": {
    "key": 84,
    "modifiers": [67108864, 134217728]
  },
  "window": {
    "alwaysOnTop": true,
    "opacity": 90,
    "x": 100,
    "y": 100,
    "width": 800,
    "height": 600
  },
  "general": {
    "autoStart": true,
    "minimizeToTray": true,
    "language": "en"
  }
}
```

## Troubleshooting

### Hotkey Not Working

1. Check if another application is using the same hotkey
2. Try a different hotkey combination in Settings
3. Restart the application

### Translator Not Loading

1. Check your internet connection
2. Click the "Retry" button in the error page
3. Try "Reload Translator" from the tray menu
4. Check if translate.yandex.ru is accessible in your browser

### Clipboard Text Not Inserting

1. Ensure you have copied text to the clipboard
2. Wait for the page to fully load before activating
3. Check if the text contains only valid characters
4. Try manually pasting (Ctrl+V) to verify clipboard content

### High Memory Usage

1. Restart the application periodically
2. Disable unnecessary browser extensions in WebView
3. Check for memory leaks and report issues

## Development

### Project Structure

```
YandexTranslator/
├── src/              # Source code
│   ├── app/         # Application core
│   ├── core/        # Core functionality (clipboard, hotkeys, network)
│   ├── ui/          # User interface
│   ├── tray/        # System tray integration
│   ├── models/       # Data models and config
│   └── resources/   # Icons and styles
├── tests/           # Unit and integration tests
└── build/           # Build output
```

### Tech Stack

- **Language**: C++17/20
- **Framework**: Qt 6.x
- **WebView**: Qt WebEngine (Chromium-based)
- **Build System**: CMake 3.16+
- **Platform**: Windows 10/11

### Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## Roadmap

- [ ] Translation history
- [ ] Multiple translation services support
- [ ] OCR (image-to-text) integration
- [ ] Voice input support
- [ ] Offline mode with cached translations
- [ ] Plugin system for extensions
- [ ] Linux and macOS support

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Qt Framework](https://www.qt.io/) for excellent cross-platform UI framework
- [Yandex Translate](https://translate.yandex.ru/) for providing the translation service
- Community contributors and testers

## Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/YandexTranslator/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/YandexTranslator/discussions)
- **Email**: support@example.com

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and updates.

---

Made with ❤️ using Qt and C++
