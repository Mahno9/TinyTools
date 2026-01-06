# Yandex Translator Desktop App - Architecture Document

## Overview

Lightweight, high-performance desktop application for Windows (10/11) that functions as an overlay over the Yandex Translate web service (https://translate.yandex.ru). The application provides a floating panel with integrated WebView for seamless translation.

**Technology Stack:**
- **Language:** C++17/20
- **Framework:** Qt 6.x (Qt Core, Qt Gui, Qt Widgets, Qt WebEngineCore, Qt WebEngineWidgets)
- **Build System:** CMake
- **Platform:** Windows 10/11 (with cross-platform capability for future Linux/macOS support)

## Architecture Principles

1. **Minimal Resource Usage:** Lightweight memory footprint (<50MB RAM)
2. **Fast Startup:** Sub-2 second application launch
3. **Responsive UI:** Smooth interactions with WebView
4. **Robust Error Handling:** Graceful degradation on network failures
5. **System Integration:** Deep Windows integration (tray, hotkeys, startup)

---

## Project Structure

```
YandexTranslator/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── Application.cpp
│   │   ├── Application.h
│   │   ├── Constants.h
│   │   └── Utils.h
│   ├── core/
│   │   ├── ClipboardManager.cpp
│   │   ├── ClipboardManager.h
│   │   ├── HotkeyManager.cpp
│   │   ├── HotkeyManager.h
│   │   └── NetworkMonitor.cpp
│   │   └── NetworkMonitor.h
│   ├── ui/
│   │   ├── MainWindow.cpp
│   │   ├── MainWindow.h
│   │   ├── SettingsDialog.cpp
│   │   ├── SettingsDialog.h
│   │   ├── WebViewContainer.cpp
│   │   └── WebViewContainer.h
│   ├── tray/
│   │   ├── TrayIcon.cpp
│   │   ├── TrayIcon.h
│   │   └── TrayMenu.h
│   ├── models/
│   │   ├── AppConfig.cpp
│   │   ├── AppConfig.h
│   │   └── TranslationRequest.h
│   └── resources/
│       ├── icons/
│       │   ├── app.ico
│       │   └── tray.ico
│       └── styles/
│           └── dark.qss
├── tests/
│   ├── unit/
│   │   ├── test_clipboard.cpp
│   │   ├── test_hotkey.cpp
│   │   └── test_config.cpp
│   └── integration/
│       └── test_webview.cpp
├── third_party/
│   └── qxt/
│       ├── qxtglobalshortcut.cpp
│       └── qxtglobalshortcut.h
└── build/
```

---

## Key Modules

### 1. Application Core (`app/`)

**Purpose:** Main application entry point and lifecycle management

**Responsibilities:**
- Application initialization
- Qt application setup
- Configuration loading
- Single instance enforcement

**Key Components:**
- `Application`: Main application class managing startup/shutdown
- `Constants`: Application-wide constants (URLs, default settings)
- `Utils`: Helper functions (file paths, string operations)

---

### 2. Clipboard Manager (`core/ClipboardManager`)

**Purpose:** Monitor and manage system clipboard operations

**Responsibilities:**
- Read clipboard text content
- Detect clipboard changes
- Filter non-text content
- Provide clipboard history (optional)

**Key Methods:**
```cpp
class ClipboardManager : public QObject {
    Q_OBJECT
public:
    explicit ClipboardManager(QObject* parent = nullptr);
    QString getText() const;
    bool hasText() const;
    void setText(const QString& text);

signals:
    void clipboardChanged(const QString& text);

private:
    QClipboard* m_clipboard;
};
```

---

### 3. Hotkey Manager (`core/HotkeyManager`)

**Purpose:** Handle global hotkey registration and processing

**Responsibilities:**
- Register global hotkeys with Windows API
- Hotkey press detection
- Custom hotkey configuration
- Hotkey conflict resolution

**Key Methods:**
```cpp
class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject* parent = nullptr);
    bool registerHotkey(int key, Qt::KeyboardModifiers modifiers);
    bool unregisterHotkey();
    void setEnabled(bool enabled);

signals:
    void hotkeyPressed();

private:
    nativeEventFilter* m_nativeFilter;
    int m_hotkeyId;
    bool m_enabled;
};
```

---

### 4. Network Monitor (`core/NetworkMonitor`)

**Purpose:** Monitor network connectivity status

**Responsibilities:**
- Detect network availability
- Notify on network status changes
- Provide offline mode handling

**Key Methods:**
```cpp
class NetworkMonitor : public QObject {
    Q_OBJECT
public:
    explicit NetworkMonitor(QObject* parent = nullptr);
    bool isOnline() const;

signals:
    void onlineStatusChanged(bool online);

private:
    QNetworkConfigurationManager* m_configManager;
};
```

---

### 5. Main Window (`ui/MainWindow`)

**Purpose:** Primary application window with floating overlay behavior

**Responsibilities:**
- Window positioning and sizing
- Always-on-top toggle
- Transparency control
- Show/hide animations
- Window state persistence

**Key Features:**
- Borderless window style
- Custom title bar (optional)
- Drag-to-move functionality
- Minimize to tray option

---

### 6. WebView Container (`ui/WebViewContainer`)

**Purpose:** Embed and manage Qt WebEngine for Yandex Translate

**Responsibilities:**
- Load Yandex Translate page
- Inject JavaScript for clipboard text insertion
- Handle page load errors
- Manage WebView lifecycle
- Optimize WebView performance

**Key Methods:**
```cpp
class WebViewContainer : public QWebEngineView {
    Q_OBJECT
public:
    explicit WebViewContainer(QWidget* parent = nullptr);
    void insertText(const QString& text);
    void reloadTranslator();
    bool isLoading() const;

signals:
    void pageLoaded(bool success);
    void loadError(const QString& error);

private:
    QWebEnginePage* m_page;
    bool m_textInserted;
};
```

---

### 7. Settings Dialog (`ui/SettingsDialog`)

**Purpose:** User interface for application configuration

**Configuration Options:**
- Global hotkey selection
- Transparency level (slider 0-100%)
- Always-on-top toggle
- Auto-start on Windows login
- Language preferences
- Window position reset

---

### 8. Tray Icon (`tray/TrayIcon`)

**Purpose:** System tray integration for background operation

**Responsibilities:**
- Display tray icon with context menu
- Handle tray icon clicks
- Show/hide window from tray
- Minimize to tray
- Display notifications

**Context Menu Actions:**
- Show/Hide Window
- Toggle Always-on-Top
- Settings...
- Reload Translator
- Check for Updates
- Exit

---

### 9. Application Config (`models/AppConfig`)

**Purpose:** Persistent configuration storage and retrieval

**Storage Format:** JSON file (settings.json) in AppData

**Configuration Properties:**
```json
{
  "hotkey": {
    "key": "Key_T",
    "modifiers": ["Ctrl", "Alt"]
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
    "language": "ru"
  }
}
```

---

## System Architecture Flow

### Application Startup Flow

```mermaid
flowchart TD
    A[Application Launch] --> B[Load Configuration]
    B --> C[Initialize Clipboard Manager]
    C --> D[Setup Network Monitor]
    D --> E[Register Global Hotkey]
    E --> F[Create Tray Icon]
    F --> G[Create Main Window]
    G --> H{Auto-start enabled?}
    H -->|Yes| I[Show Window]
    H -->|No| J[Hide to Tray]
    I --> K[Load WebView]
    J --> K
    K --> L[Enter Event Loop]
```

### Global Hotkey Activation Flow

```mermaid
flowchart TD
    A[Global Hotkey Pressed] --> B[Capture by HotkeyManager]
    B --> C[Signal: hotkeyPressed]
    C --> D{Window Visible?}
    D -->|No| E[Show Window]
    D -->|Yes| F[Hide Window]
    E --> G[Get Clipboard Text]
    F --> H[Minimize to Tray]
    G --> I{Text Available?}
    I -->|Yes| J[Insert Text to WebView]
    I -->|No| K[Focus Input Field]
    J --> L[Bring Window to Front]
    K --> L
```

### WebView Text Injection Flow

```mermaid
flowchart TD
    A[Request Insert Text] --> B{Page Loaded?}
    B -->|No| C[Wait for Load]
    C --> B
    B -->|Yes| D[Inject JavaScript]
    D --> E[Find Input Element]
    E --> F{Element Found?}
    F -->|Yes| G[Set Value via JS]
    F -->|No| H[Fallback: Try Selectors]
    H --> I{Still Not Found?}
    I -->|Yes| J[Log Error]
    I -->|No| G
    G --> K[Dispatch Input Event]
    K --> L[Trigger Translation]
    L --> M[Success Signal]
```

### Network Error Handling Flow

```mermaid
flowchart TD
    A[WebView Load Request] --> B{Network Available?}
    B -->|No| C[Show Offline Message]
    C --> D[Enable Retry Button]
    B -->|Yes| E[Load URL]
    E --> F{Load Success?}
    F -->|Yes| G[Ready for Translation]
    F -->|No| H[Show Error Dialog]
    H --> I{User Action?}
    I -->|Retry| A
    I -->|Ignore| J[Close Window]
```

---

## Implementation Details

### 1. CMake Configuration

```cmake
cmake_minimum_required(VERSION 3.16)
project(YandexTranslator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineCore
    WebEngineWidgets
    Network
)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/core/ClipboardManager.cpp
    src/core/HotkeyManager.cpp
    src/core/NetworkMonitor.cpp
    src/ui/MainWindow.cpp
    src/ui/SettingsDialog.cpp
    src/ui/WebViewContainer.cpp
    src/tray/TrayIcon.cpp
    src/models/AppConfig.cpp
)

set(HEADERS
    src/app/Application.h
    src/app/Constants.h
    src/core/ClipboardManager.h
    src/core/HotkeyManager.h
    src/core/NetworkMonitor.h
    src/ui/MainWindow.h
    src/ui/SettingsDialog.h
    src/ui/WebViewContainer.h
    src/tray/TrayIcon.h
    src/tray/TrayMenu.h
    src/models/AppConfig.h
)

add_executable(YandexTranslator WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(YandexTranslator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::Network
)

# Windows-specific settings
if(WIN32)
    set_target_properties(YandexTranslator PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "YandexTranslator"
    )
    
    # Set application icon
    set(APP_ICON_RESOURCE "${CMAKE_SOURCE_DIR}/src/resources/icons/app.rc")
    if(EXISTS ${APP_ICON_RESOURCE})
        target_sources(YandexTranslator PRIVATE ${APP_ICON_RESOURCE})
    endif()
endif()
```

### 2. Main Entry Point

```cpp
// src/main.cpp
#include "app/Application.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>

int main(int argc, char* argv[]) {
    // Enable high DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    app.setApplicationName("Yandex Translator");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("YandexTranslator");
    
    // Set dark theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    try {
        Application translatorApp;
        translatorApp.initialize();
        return app.exec();
    } catch (const std::exception& e) {
        qCritical() << "Fatal error:" << e.what();
        return -1;
    }
}
```

### 3. Application Class

```cpp
// src/app/Application.h
#pragma once
#include <QObject>
#include <QPointer>

class MainWindow;
class TrayIcon;
class HotkeyManager;
class ClipboardManager;
class NetworkMonitor;

class Application : public QObject {
    Q_OBJECT
    
public:
    explicit Application(QObject* parent = nullptr);
    ~Application();
    
    void initialize();
    
private slots:
    void onHotkeyPressed();
    void onNetworkStatusChanged(bool online);
    
private:
    void setupComponents();
    void connectSignals();
    
    QPointer<MainWindow> m_mainWindow;
    QPointer<TrayIcon> m_trayIcon;
    QPointer<HotkeyManager> m_hotkeyManager;
    QPointer<ClipboardManager> m_clipboardManager;
    QPointer<NetworkMonitor> m_networkMonitor;
};
```

```cpp
// src/app/Application.cpp
#include "Application.h"
#include "ui/MainWindow.h"
#include "tray/TrayIcon.h"
#include "core/HotkeyManager.h"
#include "core/ClipboardManager.h"
#include "core/NetworkMonitor.h"
#include "models/AppConfig.h"
#include <QDebug>

Application::Application(QObject* parent)
    : QObject(parent)
{
}

Application::~Application() {
    // Cleanup handled by QPointer
}

void Application::initialize() {
    qDebug() << "Initializing Yandex Translator...";
    
    setupComponents();
    connectSignals();
    
    // Auto-start behavior based on config
    AppConfig config;
    if (config.load()) {
        if (!config.getAutoStart()) {
            // Start hidden if auto-start is disabled
            if (m_mainWindow) {
                m_mainWindow->hide();
            }
        }
    }
    
    qDebug() << "Application initialized successfully";
}

void Application::setupComponents() {
    // Initialize clipboard manager
    m_clipboardManager = new ClipboardManager(this);
    
    // Initialize network monitor
    m_networkMonitor = new NetworkMonitor(this);
    
    // Initialize hotkey manager
    m_hotkeyManager = new HotkeyManager(this);
    
    // Register default hotkey (Ctrl+Alt+T)
    if (!m_hotkeyManager->registerHotkey(Qt::Key_T, Qt::ControlModifier | Qt::AltModifier)) {
        qWarning() << "Failed to register default hotkey";
    }
    
    // Create main window
    m_mainWindow = new MainWindow(m_clipboardManager, this);
    
    // Create tray icon
    m_trayIcon = new TrayIcon(m_mainWindow, this);
    m_trayIcon->show();
}

void Application::connectSignals() {
    // Hotkey activation
    connect(m_hotkeyManager, &HotkeyManager::hotkeyPressed,
            this, &Application::onHotkeyPressed);
    
    // Network status changes
    connect(m_networkMonitor, &NetworkMonitor::onlineStatusChanged,
            this, &Application::onNetworkStatusChanged);
    
    // Tray icon actions
    connect(m_trayIcon, &TrayIcon::showWindowRequested,
            m_mainWindow, &MainWindow::showAndActivate);
    
    connect(m_trayIcon, &TrayIcon::hideWindowRequested,
            m_mainWindow, &MainWindow::hide);
    
    connect(m_trayIcon, &TrayIcon::quitRequested,
            qApp, &QApplication::quit);
}

void Application::onHotkeyPressed() {
    if (!m_mainWindow) return;
    
    if (m_mainWindow->isVisible()) {
        // Hide window if visible
        m_mainWindow->hide();
    } else {
        // Show window and insert clipboard text
        m_mainWindow->showAndActivate();
        m_mainWindow->insertClipboardText();
    }
}

void Application::onNetworkStatusChanged(bool online) {
    if (m_mainWindow) {
        m_mainWindow->setOnlineStatus(online);
    }
    
    qInfo() << "Network status changed:" << (online ? "Online" : "Offline");
}
```

### 4. Clipboard Manager Implementation

```cpp
// src/core/ClipboardManager.h
#pragma once
#include <QObject>
#include <QClipboard>

class ClipboardManager : public QObject {
    Q_OBJECT
    
public:
    explicit ClipboardManager(QObject* parent = nullptr);
    
    QString getText() const;
    bool hasText() const;
    void setText(const QString& text);
    
    static bool isValidText(const QString& text);
    static QString trimText(const QString& text, int maxLength = 10000);
    
signals:
    void clipboardChanged(const QString& text);
    
private slots:
    void onClipboardChanged(QClipboard::Mode mode);
    
private:
    QClipboard* m_clipboard;
    QString m_lastText;
};
```

```cpp
// src/core/ClipboardManager.cpp
#include "ClipboardManager.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>

ClipboardManager::ClipboardManager(QObject* parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
{
    connect(m_clipboard, &QClipboard::dataChanged,
            this, [this]() { onClipboardChanged(QClipboard::Clipboard); });
}

QString ClipboardManager::getText() const {
    if (!m_clipboard) return QString();
    
    QString text = m_clipboard->text(QClipboard::Clipboard);
    return isValidText(text) ? text : QString();
}

bool ClipboardManager::hasText() const {
    return !getText().isEmpty();
}

void ClipboardManager::setText(const QString& text) {
    if (m_clipboard) {
        m_clipboard->setText(text, QClipboard::Clipboard);
    }
}

bool ClipboardManager::isValidText(const QString& text) {
    // Filter out empty, whitespace-only, or extremely long text
    if (text.isEmpty()) return false;
    if (text.trimmed().isEmpty()) return false;
    if (text.length() > 100000) return false; // 100KB limit
    
    return true;
}

QString ClipboardManager::trimText(const QString& text, int maxLength) {
    if (text.length() <= maxLength) return text;
    return text.left(maxLength) + "...";
}

void ClipboardManager::onClipboardChanged(QClipboard::Mode mode) {
    if (mode != QClipboard::Clipboard) return;
    
    QString text = getText();
    if (text != m_lastText && isValidText(text)) {
        m_lastText = text;
        emit clipboardChanged(text);
    }
}
```

### 5. Hotkey Manager Implementation

```cpp
// src/core/HotkeyManager.h
#pragma once
#include <QObject>
#include <QNativeEventFilter>
#include <QKeySequence>

class HotkeyManager : public QObject, public QNativeEventFilter {
    Q_OBJECT
    
public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();
    
    bool registerHotkey(int key, Qt::KeyboardModifiers modifiers);
    bool unregisterHotkey();
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
protected:
    bool nativeEventFilter(const QByteArray& eventType, 
                          void* message, 
                          qintptr* result) override;
    
signals:
    void hotkeyPressed();
    
private:
    struct HotkeyData {
        int id;
        int key;
        Qt::KeyboardModifiers modifiers;
        bool registered;
    };
    
    HotkeyData m_hotkey;
    bool m_enabled;
    static int s_hotkeyIdCounter;
};
```

```cpp
// src/core/HotkeyManager.cpp
#include "HotkeyManager.h"
#include <QApplication>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif

int HotkeyManager::s_hotkeyIdCounter = 0;

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
{
    m_hotkey.id = 0;
    m_hotkey.key = 0;
    m_hotkey.modifiers = Qt::NoModifier;
    m_hotkey.registered = false;
    
    // Install native event filter
    qApp->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager() {
    unregisterHotkey();
    qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::registerHotkey(int key, Qt::KeyboardModifiers modifiers) {
    // Unregister existing hotkey first
    if (m_hotkey.registered) {
        unregisterHotkey();
    }
    
#ifdef _WIN32
    UINT vkCode = key;
    UINT modifiersCode = 0;
    
    if (modifiers & Qt::ControlModifier) modifiersCode |= MOD_CONTROL;
    if (modifiers & Qt::AltModifier) modifiersCode |= MOD_ALT;
    if (modifiers & Qt::ShiftModifier) modifiersCode |= MOD_SHIFT;
    if (modifiers & Qt::MetaModifier) modifiersCode |= MOD_WIN;
    
    // Register hotkey with Windows
    m_hotkey.id = ++s_hotkeyIdCounter;
    BOOL result = RegisterHotKey(
        (HWND)QWidget::winId(),
        m_hotkey.id,
        modifiersCode,
        vkCode
    );
    
    if (result) {
        m_hotkey.key = key;
        m_hotkey.modifiers = modifiers;
        m_hotkey.registered = true;
        qInfo() << "Registered hotkey:" << QKeySequence(key | modifiers).toString();
        return true;
    } else {
        qWarning() << "Failed to register hotkey:" << GetLastError();
        return false;
    }
#else
    // Linux implementation using QxtGlobalShortcut
    Q_UNUSED(key);
    Q_UNUSED(modifiers);
    return false;
#endif
}

bool HotkeyManager::unregisterHotkey() {
    if (!m_hotkey.registered) return true;
    
#ifdef _WIN32
    BOOL result = UnregisterHotKey((HWND)QWidget::winId(), m_hotkey.id);
    if (result) {
        m_hotkey.registered = false;
        qInfo() << "Unregistered hotkey";
        return true;
    } else {
        qWarning() << "Failed to unregister hotkey:" << GetLastError();
        return false;
    }
#else
    return false;
#endif
}

void HotkeyManager::setEnabled(bool enabled) {
    m_enabled = enabled;
}

bool HotkeyManager::nativeEventFilter(const QByteArray& eventType, 
                                       void* message, 
                                       qintptr* result) {
#ifdef _WIN32
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_HOTKEY && m_enabled && m_hotkey.registered) {
            if (msg->wParam == m_hotkey.id) {
                emit hotkeyPressed();
                return true;
            }
        }
    }
#endif
    
    return QObject::nativeEventFilter(eventType, message, result);
}
```

### 6. Main Window Implementation

```cpp
// src/ui/MainWindow.h
#pragma once
#include <QMainWindow>
#include <QPointer>

class WebViewContainer;
class ClipboardManager;
class QSystemTrayIcon;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(ClipboardManager* clipboardManager, QWidget* parent = nullptr);
    ~MainWindow();
    
    void showAndActivate();
    void insertClipboardText();
    void setOnlineStatus(bool online);
    
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    
private slots:
    void toggleAlwaysOnTop();
    void setOpacity(int value);
    void onSettingsRequested();
    
private:
    void setupUI();
    void setupWindowFlags();
    void setupWebView();
    
    QPointer<WebViewContainer> m_webView;
    QPointer<ClipboardManager> m_clipboardManager;
    
    QPoint m_dragPosition;
    bool m_dragging;
    
    static constexpr int DEFAULT_WIDTH = 800;
    static constexpr int DEFAULT_HEIGHT = 600;
};
```

```cpp
// src/ui/MainWindow.cpp
#include "MainWindow.h"
#include "WebViewContainer.h"
#include "core/ClipboardManager.h"
#include "models/AppConfig.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(ClipboardManager* clipboardManager, QWidget* parent)
    : QMainWindow(parent)
    , m_clipboardManager(clipboardManager)
    , m_dragging(false)
{
    setupUI();
    setupWindowFlags();
    setupWebView();
    
    // Load saved position and size
    AppConfig config;
    if (config.load()) {
        resize(config.getWindowWidth(), config.getWindowHeight());
        move(config.getWindowX(), config.getWindowY());
        setWindowOpacity(config.getWindowOpacity() / 100.0);
        
        if (config.getAlwaysOnTop()) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        }
    }
}

MainWindow::~MainWindow() {
    // Save window position and size
    AppConfig config;
    if (config.load()) {
        config.setWindowWidth(width());
        config.setWindowHeight(height());
        config.setWindowX(x());
        config.setWindowY(y());
        config.save();
    }
}

void MainWindow::setupUI() {
    // Central widget
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    setCentralWidget(centralWidget);
    
    // Set initial size
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void MainWindow::setupWindowFlags() {
    // Frameless window with custom title bar behavior
    Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint;
    
    // Always on top (can be toggled)
    flags |= Qt::WindowStaysOnTopHint;
    
    setWindowFlags(flags);
    
    // Set window attributes
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
}

void MainWindow::setupWebView() {
    m_webView = new WebViewContainer(this);
    centralWidget()->layout()->addWidget(m_webView);
}

void MainWindow::showAndActivate() {
    show();
    raise();
    activateWindow();
    
    // Ensure window is visible on screen
    if (!isOnScreen()) {
        move(100, 100);
    }
}

void MainWindow::insertClipboardText() {
    if (!m_clipboardManager || !m_webView) return;
    
    QString text = m_clipboardManager->getText();
    if (!text.isEmpty()) {
        m_webView->insertText(text);
        qInfo() << "Inserted clipboard text (" << text.length() << " chars)";
    } else {
        qWarning() << "No text in clipboard";
        // Focus input field anyway
        m_webView->setFocus();
    }
}

void MainWindow::setOnlineStatus(bool online) {
    if (!m_webView) return;
    
    if (online) {
        m_webView->reloadTranslator();
    } else {
        // Show offline message in WebView
        m_webView->setHtml("<html><body style='background:#f0f0f0; "
                          "display:flex;justify-content:center;align-items:center;"
                          "height:100vh;'><h2>Network Offline</h2></body></html>");
    }
}

void MainWindow::toggleAlwaysOnTop() {
    Qt::WindowFlags flags = windowFlags();
    if (flags & Qt::WindowStaysOnTopHint) {
        flags &= ~Qt::WindowStaysOnTopHint;
    } else {
        flags |= Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();
}

void MainWindow::setOpacity(int value) {
    // Value is 0-100, convert to 0.0-1.0
    setWindowOpacity(value / 100.0);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::closeEvent(QEvent* event) {
    // Minimize to tray instead of closing
    hide();
    event->ignore();
}

void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            hide();
        }
    }
}
```

### 7. WebView Container Implementation

```cpp
// src/ui/WebViewContainer.h
#pragma once
#include <QWebEngineView>
#include <QWebEnginePage>

class WebViewContainer : public QWebEngineView {
    Q_OBJECT
    
public:
    explicit WebViewContainer(QWidget* parent = nullptr);
    
    void insertText(const QString& text);
    void reloadTranslator();
    bool isLoading() const;
    
signals:
    void pageLoaded(bool success);
    void loadError(const QString& error);
    
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    
private slots:
    void onLoadFinished(bool ok);
    void onLoadProgress(int progress);
    void onRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, 
                                   int exitCode);
    
private:
    void injectJavaScript(const QString& script);
    void waitForPageLoad();
    void findAndInsertInInputField(const QString& text);
    
    static const char* TRANSLATOR_URL;
    static const char* INPUT_SELECTOR;
};
```

```cpp
// src/ui/WebViewContainer.cpp
#include "WebViewContainer.h"
#include <QContextMenuEvent>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QDebug>

const char* WebViewContainer::TRANSLATOR_URL = "https://translate.yandex.ru/";
const char* WebViewContainer::INPUT_SELECTOR = "textarea[aria-label*='text']";

WebViewContainer::WebViewContainer(QWidget* parent)
    : QWebEngineView(parent)
{
    // Configure page
    QWebEnginePage* page = new QWebEnginePage(this);
    setPage(page);
    
    // Connect signals
    connect(page, &QWebEnginePage::loadFinished, 
            this, &WebViewContainer::onLoadFinished);
    connect(page, &QWebEnginePage::loadProgress, 
            this, &WebViewContainer::onLoadProgress);
    connect(page, &QWebEnginePage::renderProcessTerminated,
            this, &WebViewContainer::onRenderProcessTerminated);
    
    // Load translator page
    load(QUrl(TRANSLATOR_URL));
}

void WebViewContainer::insertText(const QString& text) {
    if (isLoading()) {
        qWarning() << "Cannot insert text: page is loading";
        return;
    }
    
    findAndInsertInInputField(text);
}

void WebViewContainer::reloadTranslator() {
    load(QUrl(TRANSLATOR_URL));
}

bool WebViewContainer::isLoading() const {
    return page() && page()->isLoading();
}

void WebViewContainer::onLoadFinished(bool ok) {
    if (ok) {
        qDebug() << "Page loaded successfully";
        emit pageLoaded(true);
    } else {
        qWarning() << "Page load failed";
        emit loadError("Failed to load translator page");
        emit pageLoaded(false);
    }
}

void WebViewContainer::onLoadProgress(int progress) {
    if (progress % 25 == 0) {
        qDebug() << "Loading progress:" << progress << "%";
    }
}

void WebViewContainer::onRenderProcessTerminated(
    QWebEnginePage::RenderProcessTerminationStatus status, 
    int exitCode)
{
    Q_UNUSED(exitCode);
    
    QString reason;
    switch (status) {
        case QWebEnginePage::NormalTerminationStatus:
            reason = "Normal termination";
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            reason = "Abnormal termination";
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            reason = "Render process crashed";
            break;
        case QWebEnginePage::KilledTerminationStatus:
            reason = "Render process killed";
            break;
    }
    
    qCritical() << "Render process terminated:" << reason;
    emit loadError(reason);
    
    // Attempt to reload
    QTimer::singleShot(1000, this, &WebViewContainer::reloadTranslator);
}

void WebViewContainer::findAndInsertInInputField(const QString& text) {
    // Escape text for JavaScript
    QString escapedText = text.toHtmlEscaped();
    
    QString script = QString(R"(
        (function() {
            // Try multiple selectors to find the input field
            const selectors = [
                'textarea[aria-label*="text" i]',
                'textarea[placeholder*="text" i]',
                'textarea[data-testid*="input" i]',
                '.input textarea',
                '#text-input',
                'textarea'
            ];
            
            let inputElement = null;
            
            for (const selector of selectors) {
                inputElement = document.querySelector(selector);
                if (inputElement) {
                    console.log('Found input with selector:', selector);
                    break;
                }
            }
            
            if (inputElement) {
                // Focus the input
                inputElement.focus();
                
                // Set the value
                inputElement.value = '%1';
                
                // Dispatch input event to trigger translation
                const event = new Event('input', { bubbles: true });
                inputElement.dispatchEvent(event);
                
                // Dispatch change event
                const changeEvent = new Event('change', { bubbles: true });
                inputElement.dispatchEvent(changeEvent);
                
                return { success: true, selector: selectors.join(', ') };
            } else {
                console.error('Could not find input field');
                return { success: false, error: 'Input element not found' };
            }
        })();
    )").arg(escapedText);
    
    injectJavaScript(script);
}

void WebViewContainer::injectJavaScript(const QString& script) {
    if (!page()) return;
    
    page()->runJavaScript(script, [this](const QVariant& result) {
        QVariantMap resultMap = result.toMap();
        bool success = resultMap.value("success", false).toBool();
        
        if (success) {
            qDebug() << "Text inserted successfully";
        } else {
            QString error = resultMap.value("error", "Unknown error").toString();
            qWarning() << "Failed to insert text:" << error;
        }
    });
}

void WebViewContainer::contextMenuEvent(QContextMenuEvent* event) {
    // Disable context menu for cleaner UI
    event->ignore();
}
```

### 8. Tray Icon Implementation

```cpp
// src/tray/TrayIcon.h
#pragma once
#include <QObject>
#include <QSystemTrayIcon>

class MainWindow;

class TrayIcon : public QObject {
    Q_OBJECT
    
public:
    explicit TrayIcon(MainWindow* mainWindow, QObject* parent = nullptr);
    ~TrayIcon();
    
    void showNotification(const QString& title, const QString& message);
    
signals:
    void showWindowRequested();
    void hideWindowRequested();
    void quitRequested();
    
private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowWindow();
    void onHideWindow();
    void onToggleAlwaysOnTop();
    void onOpenSettings();
    void onReloadTranslator();
    void onQuit();
    
private:
    void createContextMenu();
    void updateIconState();
    
    QSystemTrayIcon* m_trayIcon;
    QPointer<MainWindow> m_mainWindow;
    QIcon m_icon;
    QIcon m_iconActive;
};
```

```cpp
// src/tray/TrayIcon.cpp
#include "TrayIcon.h"
#include "ui/MainWindow.h"
#include <QMenu>
#include <QAction>
#include <QCoreApplication>
#include <QDebug>

TrayIcon::TrayIcon(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_trayIcon(new QSystemTrayIcon(this))
{
    // Load icons
    m_icon = QIcon(":/icons/tray.ico");
    m_iconActive = QIcon(":/icons/tray-active.ico");
    
    m_trayIcon->setIcon(m_icon);
    m_trayIcon->setToolTip("Yandex Translator");
    
    createContextMenu();
    
    // Connect activation signal
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &TrayIcon::onActivated);
}

TrayIcon::~TrayIcon() {
    m_trayIcon->hide();
}

void TrayIcon::show() {
    m_trayIcon->show();
}

void TrayIcon::createContextMenu() {
    QMenu* menu = new QMenu();
    
    // Show/Hide Window
    QAction* showAction = menu->addAction("Show Window");
    connect(showAction, &QAction::triggered, this, &TrayIcon::onShowWindow);
    
    QAction* hideAction = menu->addAction("Hide Window");
    connect(hideAction, &QAction::triggered, this, &TrayIcon::onHideWindow);
    
    menu->addSeparator();
    
    // Toggle Always on Top
    QAction* toggleTopAction = menu->addAction("Toggle Always on Top");
    connect(toggleTopAction, &QAction::triggered, 
            this, &TrayIcon::onToggleAlwaysOnTop);
    
    menu->addSeparator();
    
    // Settings
    QAction* settingsAction = menu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, &TrayIcon::onOpenSettings);
    
    // Reload Translator
    QAction* reloadAction = menu->addAction("Reload Translator");
    connect(reloadAction, &QAction::triggered, this, &TrayIcon::onReloadTranslator);
    
    menu->addSeparator();
    
    // Quit
    QAction* quitAction = menu->addAction("Exit");
    connect(quitAction, &QAction::triggered, this, &TrayIcon::onQuit);
    
    m_trayIcon->setContextMenu(menu);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            // Single click: toggle window visibility
            if (m_mainWindow && m_mainWindow->isVisible()) {
                onHideWindow();
            } else {
                onShowWindow();
            }
            break;
            
        case QSystemTrayIcon::DoubleClick:
            // Double click: show window
            onShowWindow();
            break;
            
        default:
            break;
    }
}

void TrayIcon::onShowWindow() {
    if (m_mainWindow) {
        m_mainWindow->showAndActivate();
        emit showWindowRequested();
    }
}

void TrayIcon::onHideWindow() {
    if (m_mainWindow) {
        m_mainWindow->hide();
        emit hideWindowRequested();
    }
}

void TrayIcon::onToggleAlwaysOnTop() {
    if (m_mainWindow) {
        m_mainWindow->toggleAlwaysOnTop();
    }
}

void TrayIcon::onOpenSettings() {
    // TODO: Implement settings dialog
    qInfo() << "Opening settings...";
}

void TrayIcon::onReloadTranslator() {
    // TODO: Implement reload
    qInfo() << "Reloading translator...";
}

void TrayIcon::onQuit() {
    emit quitRequested();
    QCoreApplication::quit();
}

void TrayIcon::showNotification(const QString& title, const QString& message) {
    m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
}

void TrayIcon::updateIconState() {
    if (m_mainWindow && m_mainWindow->isVisible()) {
        m_trayIcon->setIcon(m_iconActive);
    } else {
        m_trayIcon->setIcon(m_icon);
    }
}
```

### 9. Application Config Implementation

```cpp
// src/models/AppConfig.h
#pragma once
#include <QString>
#include <QJsonObject>

class AppConfig {
public:
    AppConfig();
    
    bool load();
    bool save();
    void resetToDefaults();
    
    // Hotkey settings
    int getHotkeyKey() const;
    Qt::KeyboardModifiers getHotkeyModifiers() const;
    void setHotkey(int key, Qt::KeyboardModifiers modifiers);
    
    // Window settings
    bool getAlwaysOnTop() const;
    void setAlwaysOnTop(bool value);
    
    int getWindowOpacity() const; // 0-100
    void setWindowOpacity(int value);
    
    int getWindowX() const;
    void setWindowX(int value);
    
    int getWindowY() const;
    void setWindowY(int value);
    
    int getWindowWidth() const;
    void setWindowWidth(int value);
    
    int getWindowHeight() const;
    void setWindowHeight(int value);
    
    // General settings
    bool getAutoStart() const;
    void setAutoStart(bool value);
    
    bool getMinimizeToTray() const;
    void setMinimizeToTray(bool value);
    
    QString getLanguage() const;
    void setLanguage(const QString& value);
    
private:
    QString getConfigFilePath() const;
    
    QJsonObject m_config;
    QString m_configPath;
};
```

```cpp
// src/models/AppConfig.cpp
#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

AppConfig::AppConfig() {
    m_configPath = getConfigFilePath();
    resetToDefaults();
}

bool AppConfig::load() {
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file for reading:" << m_configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Config file parse error:" << error.errorString();
        return false;
    }
    
    m_config = doc.object();
    qInfo() << "Configuration loaded from:" << m_configPath;
    return true;
}

bool AppConfig::save() {
    QJsonDocument doc(m_config);
    
    // Ensure directory exists
    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Cannot create config directory:" << dir.path();
            return false;
        }
    }
    
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open config file for writing:" << m_configPath;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qInfo() << "Configuration saved to:" << m_configPath;
    return true;
}

void AppConfig::resetToDefaults() {
    // Hotkey: Ctrl+Alt+T
    QJsonObject hotkey;
    hotkey["key"] = static_cast<int>(Qt::Key_T);
    QJsonArray modifiers;
    modifiers.append(static_cast<int>(Qt::ControlModifier));
    modifiers.append(static_cast<int>(Qt::AltModifier));
    hotkey["modifiers"] = modifiers;
    m_config["hotkey"] = hotkey;
    
    // Window settings
    QJsonObject window;
    window["alwaysOnTop"] = true;
    window["opacity"] = 90;
    window["x"] = 100;
    window["y"] = 100;
    window["width"] = 800;
    window["height"] = 600;
    m_config["window"] = window;
    
    // General settings
    QJsonObject general;
    general["autoStart"] = true;
    general["minimizeToTray"] = true;
    general["language"] = "en";
    m_config["general"] = general;
}

int AppConfig::getHotkeyKey() const {
    QJsonObject hotkey = m_config["hotkey"].toObject();
    return hotkey["key"].toInt();
}

Qt::KeyboardModifiers AppConfig::getHotkeyModifiers() const {
    QJsonObject hotkey = m_config["hotkey"].toObject();
    QJsonArray modifiersArray = hotkey["modifiers"].toArray();
    
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    for (const QJsonValue& value : modifiersArray) {
        modifiers |= static_cast<Qt::KeyboardModifier>(value.toInt());
    }
    
    return modifiers;
}

void AppConfig::setHotkey(int key, Qt::KeyboardModifiers modifiers) {
    QJsonObject hotkey;
    hotkey["key"] = key;
    
    QJsonArray modifiersArray;
    if (modifiers & Qt::ControlModifier)
        modifiersArray.append(static_cast<int>(Qt::ControlModifier));
    if (modifiers & Qt::AltModifier)
        modifiersArray.append(static_cast<int>(Qt::AltModifier));
    if (modifiers & Qt::ShiftModifier)
        modifiersArray.append(static_cast<int>(Qt::ShiftModifier));
    if (modifiers & Qt::MetaModifier)
        modifiersArray.append(static_cast<int>(Qt::MetaModifier));
    
    hotkey["modifiers"] = modifiersArray;
    m_config["hotkey"] = hotkey;
}

bool AppConfig::getAlwaysOnTop() const {
    return m_config["window"].toObject()["alwaysOnTop"].toBool(true);
}

void AppConfig::setAlwaysOnTop(bool value) {
    m_config["window"].toObject()["alwaysOnTop"] = value;
}

int AppConfig::getWindowOpacity() const {
    return m_config["window"].toObject()["opacity"].toInt(90);
}

void AppConfig::setWindowOpacity(int value) {
    m_config["window"].toObject()["opacity"] = qBound(20, value, 100);
}

int AppConfig::getWindowX() const {
    return m_config["window"].toObject()["x"].toInt(100);
}

void AppConfig::getWindowX(int value) {
    m_config["window"].toObject()["x"] = value;
}

int AppConfig::getWindowY() const {
    return m_config["window"].toObject()["y"].toInt(100);
}

void AppConfig::setWindowY(int value) {
    m_config["window"].toObject()["y"] = value;
}

int AppConfig::getWindowWidth() const {
    return m_config["window"].toObject()["width"].toInt(800);
}

void AppConfig::setWindowWidth(int value) {
    m_config["window"].toObject()["width"] = qBound(400, value, 1920);
}

int AppConfig::getWindowHeight() const {
    return m_config["window"].toObject()["height"].toInt(600);
}

void AppConfig::setWindowHeight(int value) {
    m_config["window"].toObject()["height"] = qBound(300, value, 1080);
}

bool AppConfig::getAutoStart() const {
    return m_config["general"].toObject()["autoStart"].toBool(true);
}

void AppConfig::setAutoStart(bool value) {
    m_config["general"].toObject()["autoStart"] = value;
}

bool AppConfig::getMinimizeToTray() const {
    return m_config["general"].toObject()["minimizeToTray"].toBool(true);
}

void AppConfig::setMinimizeToTray(bool value) {
    m_config["general"].toObject()["minimizeToTray"] = value;
}

QString AppConfig::getLanguage() const {
    return m_config["general"].toObject()["language"].toString("en");
}

void AppConfig::setLanguage(const QString& value) {
    m_config["general"].toObject()["language"] = value;
}

QString AppConfig::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return configDir + "/settings.json";
}
```

---

## Error Handling Strategy

### 1. WebView Load Errors

```cpp
// In WebViewContainer
void WebViewContainer::onLoadFinished(bool ok) {
    if (!ok) {
        QString errorHtml = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        display: flex;
                        flex-direction: column;
                        align-items: center;
                        justify-content: center;
                        height: 100vh;
                        margin: 0;
                        background-color: #f5f5f5;
                    }
                    .error-container {
                        text-align: center;
                        padding: 40px;
                    }
                    h1 { color: #d32f2f; }
                    p { color: #666; }
                    button {
                        background-color: #1976d2;
                        color: white;
                        border: none;
                        padding: 10px 20px;
                        font-size: 16px;
                        cursor: pointer;
                        border-radius: 4px;
                        margin-top: 20px;
                    }
                    button:hover { background-color: #1565c0; }
                </style>
            </head>
            <body>
                <div class="error-container">
                    <h1>Failed to Load Translator</h1>
                    <p>Please check your internet connection and try again.</p>
                    <button onclick="window.location.reload()">Retry</button>
                </div>
            </body>
            </html>
        )";
        
        setHtml(errorHtml);
        emit loadError("Failed to load translator page");
    }
}
```

### 2. Network Monitoring

```cpp
// In NetworkMonitor
void NetworkMonitor::onConfigurationChanged(const QNetworkConfiguration& config) {
    bool online = config.state() == QNetworkConfiguration::Active;
    
    if (online != m_isOnline) {
        m_isOnline = online;
        emit onlineStatusChanged(online);
        
        if (!online) {
            qWarning() << "Network connection lost";
        } else {
            qInfo() << "Network connection restored";
        }
    }
}
```

### 3. Clipboard Access Errors

```cpp
// In ClipboardManager
QString ClipboardManager::getText() const {
    try {
        if (!m_clipboard) {
            throw std::runtime_error("Clipboard not initialized");
        }
        
        QString text = m_clipboard->text(QClipboard::Clipboard);
        
        if (!isValidText(text)) {
            throw std::runtime_error("Invalid clipboard content");
        }
        
        return text;
        
    } catch (const std::exception& e) {
        qWarning() << "Error reading clipboard:" << e.what();
        return QString();
    }
}
```

---

## Performance Optimizations

### 1. WebView Performance

```cpp
// In WebViewContainer constructor
void WebViewContainer::setupPerformanceSettings() {
    // Enable hardware acceleration
    QWebEngineSettings* settings = page()->settings();
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, false); // Not needed
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    
    // Reduce memory usage
    settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::PrintElementBackgrounds, false);
}
```

### 2. Lazy Loading

```cpp
// In Application::initialize()
void Application::initialize() {
    setupComponents();
    
    // Load WebView only when needed
    if (!m_mainWindow) {
        m_mainWindow = new MainWindow(m_clipboardManager, this);
    }
    
    // Don't load WebView until window is shown
    connect(m_mainWindow, &MainWindow::windowShown, [this]() {
        if (m_mainWindow) {
            m_mainWindow->loadWebView();
        }
    });
}
```

---

## Build Instructions

### Prerequisites

1. **Qt 6.x**: Install from https://www.qt.io/download
2. **CMake 3.16+**: Install from https://cmake.org/download/
3. **Visual Studio 2019+** (with C++ development tools) or MinGW-w64
4. **C++17 compiler**

### Build Steps

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.3.0/msvc2019_64" ..

# Build
cmake --build . --config Release

# Run
./Release/YandexTranslator.exe
```

### Deployment

Use **windeployqt** to bundle required Qt libraries:

```bash
windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw ./Release/YandexTranslator.exe
```

Or create an installer with **NSIS** or **Inno Setup**.

---

## Testing Strategy

### Unit Tests

```cpp
// tests/unit/test_clipboard.cpp
#include <QtTest>
#include "core/ClipboardManager.h"

class TestClipboard : public QObject {
    Q_OBJECT
    
private slots:
    void testGetEmptyText();
    void testSetText();
    void testIsValidText();
};

void TestClipboard::testGetEmptyText() {
    ClipboardManager manager;
    QVERIFY(manager.getText().isEmpty());
}

void TestClipboard::testSetText() {
    ClipboardManager manager;
    manager.setText("Test text");
    QCOMPARE(manager.getText(), QString("Test text"));
}

void TestClipboard::testIsValidText() {
    QVERIFY(ClipboardManager::isValidText("Valid text"));
    QVERIFY(!ClipboardManager::isValidText(""));
    QVERIFY(!ClipboardManager::isValidText("   "));
    QVERIFY(!ClipboardManagerManager::isValidText(QString().fill('a', 100001)));
}

QTEST_MAIN(TestClipboard)
#include "test_clipboard.moc"
```

---

## Security Considerations

1. **HTTPS Only**: Always use HTTPS for translator URL
2. **Input Validation**: Sanitize clipboard text before JavaScript injection
3. **No External Scripts**: Disable JavaScript from external sources
4. **Content Security Policy**: Configure WebView CSP headers
5. **Update Mechanism**: Secure update verification with code signing

---

## Future Enhancements

1. **Translation History**: Store recent translations locally
2. **Multiple Languages**: Support for other translation services
3. **OCR Integration**: Text extraction from images
4. **Voice Input**: Speech-to-text for translation
5. **Offline Mode**: Cached translations for offline use
6. **Plugin System**: Extensible architecture for custom features

---

## Conclusion

This architecture provides a solid foundation for a lightweight, high-performance desktop translator application using C++ and Qt. The modular design allows for easy maintenance, testing, and future enhancements while maintaining excellent resource efficiency and user experience.

Key advantages:
- **Minimal memory footprint** (~30-50MB)
- **Fast startup** (<2 seconds)
- **Native Windows integration**
- **Cross-platform potential**
- **Robust error handling**
- **Clean, maintainable code**
