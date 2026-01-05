# Architecture Summary - Yandex Translator Desktop App

## Quick Reference

### Technology Stack
- **Language**: C++17/20
- **Framework**: Qt 6.x (Qt Core, Gui, Widgets, WebEngine)
- **Build System**: CMake 3.16+
- **Platform**: Windows 10/11 (with cross-platform capability)

### Key Features
✅ Global hotkey activation (Ctrl+Alt+T)
✅ Automatic clipboard text insertion
✅ Floating overlay window with transparency
✅ Always-on-top mode toggle
✅ System tray integration
✅ Auto-start on Windows login
✅ Network status monitoring
✅ Offline error handling

### Performance Targets
- **Memory Usage**: ~30-50 MB
- **Startup Time**: <2 seconds
- **Distribution Size**: ~50-100 MB (with dependencies)

## Project Structure

```
YandexTranslator/
├── src/
│   ├── app/              # Application core & lifecycle
│   ├── core/             # Clipboard, Hotkey, Network managers
│   ├── ui/               # MainWindow, WebView, Settings
│   ├── tray/             # System tray integration
│   ├── models/           # AppConfig, data models
│   └── resources/        # Icons, styles
├── tests/                # Unit & integration tests
└── build/                # Build output
```

## Core Components

### 1. Application Layer
- **Application**: Main controller managing all components
- **AppConfig**: JSON-based configuration storage
- **Constants**: Application-wide constants

### 2. Core Functionality
- **ClipboardManager**: Monitor and manage clipboard
- **HotkeyManager**: Global hotkey registration (Windows API)
- **NetworkMonitor**: Network connectivity detection

### 3. User Interface
- **MainWindow**: Floating overlay window
- **WebViewContainer**: Qt WebEngine wrapper for translator
- **SettingsDialog**: User configuration interface

### 4. System Integration
- **TrayIcon**: System tray icon with context menu
- **Auto-start**: Windows registry integration

## Key Workflows

### Hotkey Activation
```
Global Hotkey Pressed
  ↓
HotkeyManager (Windows API)
  ↓
Application::onHotkeyPressed()
  ↓
MainWindow::showAndActivate()
  ↓
ClipboardManager::getText()
  ↓
WebViewContainer::insertText()
  ↓
JavaScript Injection (DOM manipulation)
  ↓
Translation Result
```

### Error Handling
```
WebView Load Request
  ↓
Network Check
  ↓
{ Online? }
  ├─ No → Show Offline Message
  └─ Yes → Load URL
  ↓
{ Success? }
  ├─ No → Show Error Dialog + Retry Option
  └─ Yes → Ready for Translation
```

## Implementation Highlights

### Global Hotkey (Windows API)
```cpp
BOOL result = RegisterHotKey(
    (HWND)QWidget::winId(),
    hotkeyId,
    modifiersCode,  // MOD_CONTROL | MOD_ALT
    vkCode         // Key code
);
```

### Clipboard Text Injection (JavaScript)
```javascript
// Find input element
const input = document.querySelector('textarea[aria-label*="text"]');
input.value = clipboardText;

// Trigger translation
input.dispatchEvent(new Event('input', { bubbles: true }));
```

### WebView Performance
```cpp
settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
settings->setAttribute(QWebEngineSettings::WebGLEnabled, false);
settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
```

## Configuration Storage

**Location**: `%APPDATA%\YandexTranslator\settings.json`

**Structure**:
```json
{
  "hotkey": { "key": 84, "modifiers": [67108864, 134217728] },
  "window": {
    "alwaysOnTop": true,
    "opacity": 90,
    "x": 100, "y": 100,
    "width": 800, "height": 600
  },
  "general": {
    "autoStart": true,
    "minimizeToTray": true,
    "language": "en"
  }
}
```

## Build Commands

### Configure
```bash
cmake -G "Visual Studio 16 2019" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64" ^
  -DCMAKE_BUILD_TYPE=Release ^
  ..
```

### Build
```bash
cmake --build . --config Release
```

### Deploy
```bash
windeployqt --release --no-translations Release/YandexTranslator.exe
```

## Documentation Files

1. **[architecture.md](architecture.md)** - Complete architecture with detailed code examples
2. **[README.md](README.md)** - User-facing documentation with installation and usage
3. **[BUILD.md](BUILD.md)** - Detailed build instructions and deployment
4. **[DEVELOPER.md](DEVELOPER.md)** - Developer guide with coding standards and testing
5. **SUMMARY.md** (this file) - Quick reference and overview

## Next Steps

To implement this architecture:

1. **Review the architecture document** for detailed implementation
2. **Set up development environment** (Qt 6.x, CMake, Visual Studio)
3. **Create project structure** following the outlined directories
4. **Implement core modules** in order:
   - Application & AppConfig
   - ClipboardManager
   - HotkeyManager
   - MainWindow & WebViewContainer
   - TrayIcon & Settings
5. **Add error handling** as specified
6. **Write unit tests** for each module
7. **Build and test** on Windows 10/11
8. **Package for distribution** using windeployqt

## Advantages of This Architecture

### Performance
- Native C++ code for maximum speed
- Qt WebEngine (Chromium-based) for fast rendering
- Minimal dependencies and overhead
- Efficient memory management with Qt's parent-child system

### Maintainability
- Clear separation of concerns
- Modular design with well-defined interfaces
- Comprehensive error handling
- Extensive documentation and code comments

### Extensibility
- Plugin-ready architecture for future features
- Cross-platform foundation (Linux/macOS support possible)
- Easy to add new translation services
- Configuration-driven behavior

### User Experience
- Fast startup and responsive UI
- Minimal resource usage
- Intuitive hotkey workflow
- Robust error handling with graceful degradation

## Questions?

If you have questions about the architecture or need clarification on any aspect:

1. Review the detailed [architecture.md](architecture.md) document
2. Check the [DEVELOPER.md](DEVELOPER.md) for implementation details
3. Refer to [BUILD.md](BUILD.md) for setup and build instructions

---

**Ready to implement?** Switch to Code mode to start building the application!
