# TinyTools - Architecture Document

## Overview

**TinyTools** is a lightweight, high-performance desktop overlay application for Windows 10/11. It facilitates quick access to multiple web-based tools (translators, chatbots, documenation) through a single, global hotkey-activated interface. It uses a persistent "floating" window with transparency and integrated hotkey management.

**Technology Stack:**
- **Language:** C++17/20
- **Framework:** Qt 6.x (Core, Gui, Widgets, WebEngine, Network)
- **Build System:** CMake
- **Platform:** Windows 10/11 (Primary), extensible to Linux/macOS

## Architecture Principles

1.  **Multi-Resource Support**: The core architectural concept is the `WebResource` model, allowing $N$ configuratble tools.
2.  **Singleton Resource Management**: A central `ResourceManager` handles persistence, CRUD operations, and resource ordering.
3.  **Low Latency**: The `WebViewContainer` is kept alive (or lazily loaded) to ensure instant visibility upon hotkey press.
4.  **Extensible Hotkey System**: A generalized `HotkeyManager` supports registering dynamic keys for each resource.

---

## Project Structure

```
TinyTools/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp                 # Entry point
│   ├── app/
│   │   ├── Application.cpp      # Main application controller
│   │   └── Constants.h          # Global constants
│   ├── core/
│   │   ├── ClipboardManager.cpp # System clipboard monitoring
│   │   ├── HotkeyManager.cpp    # Global hotkey hook & routing
│   │   └── NetworkMonitor.cpp   # Online/Offline detection
│   ├── models/
│   │   ├── WebResource.h        # Data struct for a single tool
│   │   ├── ResourceManager.cpp  # Manager for list of resources
│   │   └── AppConfig.cpp        # Low-level settings persistence
│   ├── ui/
│   │   ├── MainWindow.cpp       # The floating overlay window
│   │   ├── WebViewContainer.cpp # QWebEngineView wrapper
│   │   └── SettingsDialog.cpp   # Configuration UI
│   └── tray/
│       └── TrayIcon.cpp         # System tray integration
└── build/
```

---

## Key Modules

### 1. Data Models (`models/`)

#### `WebResource`
A struct representing a single tool.
- **Properties**: `id` (UUID), `name`, `url`, `icon`, `order`, `isEnabled`.
- **Scripts**: `openScript` (JS executed on normal open), `altOpenScript` (JS executed on alternative hotkey).
- **Hotkeys**: `openHotkey`, `altOpenHotkey` (key codes + modifiers).

#### `ResourceManager` (Singleton)
- Manages the list of `WebResource` objects.
- Handles saving/loading from `AppConfig` (JSON).
- Signals: `resourcesChanged`, `activeResourceChanged`.

### 2. Core Service (`core/`)

#### `HotkeyManager`
- Registers global system hotkeys (using Windows API or platform-specific plugins).
- Supports dynamic registration/unregistration at runtime when user changes settings.
- Maps unique integer IDs to resource actions.

### 3. UI Layer (`ui/`)

#### `MainWindow`
- **Floating Overlay**: Frameless, transparent, always-on-top.
- **Tabbed Interface**: `QTabBar` allows switching between active resources.
- **Logic**: 
    - Listens to `ResourceManager` signals to rebuild tabs.
    - Listens to `HotkeyManager` to toggle visibility or switch tabs.
    - Handles "Drag to move" logic for frameless window.

#### `WebViewContainer`
- Wraps `QWebEngineView`.
- **Dynamic Loading**: `loadResource(WebResource)` switches the current page.
- **Script Injection**: Executes the `WebResource.openScript` immediately after page load to handle tasks like "Insert Clipboard Text".

#### `SettingsDialog`
- **General Tab**: Window opacity, auto-start, hotkey configuration.
- **Resources Tab**: 
    - List of collapsible panels (`ResourcePanel`) for editing resources.
    - Add/Remove/Reorder functionality.
    - Import/Export presets to JSON.

---

## Data Flow

### 1. Startup
1. `Application::initialize()` loads `AppConfig`.
2. `ResourceManager::loadFromConfig()` explicitly loads resources from JSON.
3. `MainWindow` is created.
4. `MainWindow` subscribes to `ResourceManager::resourcesChanged`.
5. `MainWindow::refreshResources()` is called, populating the `QTabBar`.
6. First tab (or last used) is selected, triggering `loadCurrentResource()`.

### 2. Hotkey Activation
1. User presses global hotkey (e.g., `Alt+Shift+T`).
2. `HotkeyManager` detects event via native filter.
3. Emits `hotkeyPressed(type, id)`.
4. `Application` receives signal:
    - If **Main Toggle**: Shows/Hides `MainWindow`.
    - If **Resource Hotkey**: Switches `MainWindow` to specific resource ID and Shows it.
5. If "Show and Insert" logic is active, `MainWindow` grabs clipboard text via `ClipboardManager` and calls `WebViewContainer::executeScript()`.

### 3. Resource Injection
1. `WebViewContainer` finishes loading URL.
2. Checks if `m_openScript` is defined for current resource.
3. If text insertion is requested, replaces `%1` placeholder in script with escaped clipboard text.
4. Runs `page()->runJavaScript()`.

---

## Configuration File (`settings.json`)

```json
{
  "general": {
    "alwaysOnTop": true,
    "opacity": 90,
    "autoStart": false
  },
  "hotkeys": {
    "mainToggle": { "key": 84, "modifiers": 134217728 }
  },
  "startup": {
    "mode": "lastUsed",
    "lastUsedResourceId": "uuid-..."
  },
  "resources": [
    {
      "id": "uuid-1",
      "name": "Google Translate",
      "url": "https://translate.google.com",
      "openScript": "...",
      "openHotkey": { "key": 71, "modifiers": 134217728 } 
    }
  ]
}
```
