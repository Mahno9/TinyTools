# TinyTools

A versatile, lightweight desktop utility for Windows (10/11) that provides quick access to web resources (translators, AI tools, documentation) via a floating overlay window using global hotkeys.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)

## Features

✨ **Lightweight & Fast**
- Minimal memory footprint (~30-50MB RAM)
- Sub-2 second startup time
- Efficient Qt WebEngine integration (Chromium-based)

🔗 **Multi-Resource Support**
- Add unlimited web resources (e.g., Google Translate, DeepL, ChatGPT, Stack Overflow)
- Switch between resources instantly using Tabs or Hotkeys (`Alt+1`, `Alt+2`, etc.)
- Configure custom JavaScript to execute on load for each resource

🎯 **Global Hotkeys**
- **Main Toggle**: Toggle window visibility (Default: `Alt+Shift+T`)
- **Alternative Toggle**: Open window and execute a secondary script (e.g., auto-translate clipboard) (Default: `Ctrl+Alt+Shift+T`)
- **Resource-Specific Hotkeys**: Map unique hotkeys to open specific tools instantly (e.g., `Alt+G` for Google, `Alt+D` for DeepL)

🪟 **Floating Overlay**
- Always-on-top mode toggle
- Adjustable transparency (20-100%)
- Drag-to-move functionality
- Frameless, minimalist design

📋 **Smart Clipboard Integration**
- Auto-insert clipboard text into the active resource
- Customizable text injection scripts (JavaScript) per resource

⚙️ **System Integration**
- Minimize to system tray
- Auto-start on Windows login
- JSON-based preset import/export for easy sharing of configurations

## Screenshots

*Add screenshots here*

## System Requirements

- **OS**: Windows 10 or 11 (64-bit)
- **RAM**: 512MB minimum, 2GB recommended
- **Disk**: 50MB free space (+ Qt dependencies)
- **Network**: Internet connection for web resources

## Usage

### Quick Start

1. **Launch the application** - It will start in the system tray.
2. **Press Alt+Shift+T** to show the overlay window.
3. **Add Resources**:
    - Click the "Gear" icon ⚙️ to open Settings.
    - Go to the "Resources" tab.
    - Click "Add Resource".
    - Enter a Name (e.g., "Google Translate") and URL (e.g., `https://translate.google.com`).
    - (Optional) Configure "Open Script" to auto-paste text: `document.querySelector('textarea').value = '%1';`
    - Click "Apply".
4. **Use Tabs**: Switch between tools using the tabs at the top or `Alt+1`, `Alt+2`.

### Hotkeys

| Shortcut | Action |
|----------|--------|
| `Alt+Shift+T` | Toggle Main Window (showing last used tab) |
| `Ctrl+Alt+Shift+T` | Alternative Toggle (executes alternate JS logic) |
| `Alt+[1-9]` | Switch to Resource Tab 1-9 (when window is active) |
| [Custom] | Resource-specific toggle (configured in Settings) |

## Configuration

Configuration is stored in `%APPDATA%\TinyTools\settings.json`.

### Example Resource Config (JSON)

```json
{
  "resources": [
    {
      "id": "uuid-string",
      "name": "Yandex.Translate",
      "url": "https://translate.yandex.ru/",
      "openScript": "const input = document.querySelector('#fakeArea'); if(input) { input.value = '%1'; input.dispatchEvent(new Event('input', { bubbles: true })); }",
      "openHotkey": { "key": 89, "modifiers": 134217728 } // Alt+Y
    }
  ]
}
```

## Development

### Project Structure

```
TinyTools/
├── src/              # Source code
│   ├── app/         # Application core (startup, lifecycle)
│   ├── core/        # Core managers (Clipboard, Hotkey, Network)
│   ├── ui/          # UI components (MainWindow, SettingsDialog, WebView)
│   ├── models/      # Data models (WebResource, ResourceManager)
│   └── tray/        # System tray integration
├── tests/           # Unit tests
└── build/           # Build output
```

### Build Instructions

See [BUILD.md](plans/BUILD.md) for detailed build instructions using CMake and Qt 6.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Qt Framework** for the robust cross-platform UI engine.
- **Yandex, Google, DeepL** and others for the web services we love to use.
