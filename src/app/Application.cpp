#include "Application.h"
#include "../core/ClipboardManager.h"
#include "../core/HotkeyManager.h"
#include "../core/NetworkMonitor.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "../models/WebResource.h"
#include "../tray/TrayIcon.h"
#include "../ui/MainWindow.h"
#include <QApplication>
#include <QDebug>

Application::Application(QObject *parent) : QObject(parent) {
  qDebug() << "Application::Application() - ENTRY";
  qDebug() << "Application instance constructed with parent:"
           << (parent ? "yes" : "no");
  qDebug() << "Application::Application() - EXIT";
}

Application::~Application() {
  qDebug() << "Application::~Application() - ENTRY";
  // Delete MainWindow first: its WebViewContainers/QWebEnginePages must be gone
  // before QApplication (later) destroys the WebEngine profile (its child).
  delete m_mainWindow.data();
  // Explicitly delete singletons while QApplication is still alive.
  ResourceManager::cleanupInstance();
  AppConfig::cleanupInstance();
  qDebug() << "Application::~Application() - EXIT";
}

void Application::initialize() {
  qDebug() << "Application::initialize() - ENTRY";
  qDebug() << "Initializing TinyTools application components...";

  try {
    qDebug() << "Step 1: Loading configuration...";
    if (!AppConfig::instance()->load()) {
      qWarning() << "Failed to load configuration - using defaults";
      qDebug() << "Using default configuration values";
    } else {
      qDebug() << "Configuration loaded successfully";
      qDebug() << "Show window on startup:"
               << (AppConfig::instance()->getShowWindowOnStartup()
                       ? "enabled"
                       : "disabled");
      qDebug() << "Auto-start on login:"
               << (AppConfig::instance()->getAutoStartOnLogin() ? "enabled"
                                                                : "disabled");
      qDebug() << "Window size:" << AppConfig::instance()->getWindowWidth()
               << "x" << AppConfig::instance()->getWindowHeight();
      qDebug() << "Window position:" << AppConfig::instance()->getWindowX()
               << "," << AppConfig::instance()->getWindowY();
      qDebug() << "Always on top:"
               << (AppConfig::instance()->getAlwaysOnTop() ? "yes" : "no");
      qDebug() << "Window opacity:" << AppConfig::instance()->getWindowOpacity()
               << "%";
      qDebug() << "Main hotkey key:"
               << AppConfig::instance()->getHotkeyKey(HotkeyType::MainToggle);
      qDebug() << "Main hotkey modifiers:"
               << QKeySequence(AppConfig::instance()->getHotkeyKey(
                                   HotkeyType::MainToggle) |
                               AppConfig::instance()->getHotkeyModifiers(
                                   HotkeyType::MainToggle))
                      .toString();
    }

    qDebug() << "Step 1 complete: Configuration loaded";

    // Fix: Explicitly load resources from config on startup
    qDebug() << "Step 1.5: Loading resources...";
    ResourceManager::instance()->loadFromConfig();
    qDebug() << "Loaded " << ResourceManager::instance()->getResourceCount()
             << " resources";

    qDebug() << "Step 2: Setting up components...";
    setupComponents();
    qDebug() << "Step 2 complete: Components setup finished";

    qDebug() << "Step 3: Connecting signals...";
    connectSignals();
    qDebug() << "Step 3 complete: Signals connected";

    qDebug() << "Step 4: Applying startup settings...";
    bool showWindow = AppConfig::instance()->getShowWindowOnStartup();
    bool autoStartOnLogin =
        AppConfig::instance()->isAutoStartEnabledInRegistry();

    qDebug() << "Show window on startup:" << (showWindow ? "YES" : "NO");
    qDebug() << "Auto-start on login (registry):"
             << (autoStartOnLogin ? "YES" : "NO");

    if (m_mainWindow) {
      if (showWindow) {
        qDebug() << "Showing main window";
        m_mainWindow->show();
        qDebug() << "Main window shown successfully";
      } else {
        qDebug() << "Hiding main window (running in background)";
        m_mainWindow->hide();
        qDebug() << "Main window hidden successfully";
      }
    } else {
      qWarning() << "Cannot set window visibility - m_mainWindow is null";
    }
    qDebug() << "Step 4 complete: Startup settings applied";

    qDebug() << "Application initialized successfully";
    qDebug() << "Application::initialize() - EXIT";
  } catch (const std::exception &e) {
    qCritical() << "Application::initialize() - EXCEPTION: Standard exception:"
                << e.what();
    qCritical() << "Application::initialize() - EXIT with error";
    throw;
  } catch (...) {
    qCritical()
        << "Application::initialize() - EXCEPTION: Unknown exception caught";
    qCritical() << "Application::initialize() - EXIT with error";
    throw;
  }
}

void Application::setupComponents() {
  qDebug() << "Application::setupComponents() - ENTRY";
  qDebug() << "Starting component initialization sequence...";

  // Initialize clipboard manager
  qDebug() << "[Component 1/5] Creating ClipboardManager...";
  m_clipboardManager = new ClipboardManager(this);
  qDebug() << "[Component 1/5] ClipboardManager created successfully";

  // Initialize network monitor
  qDebug() << "[Component 2/5] Creating NetworkMonitor...";
  m_networkMonitor = new NetworkMonitor(this);
  qDebug() << "[Component 2/5] NetworkMonitor created successfully";

  // Initialize hotkey manager
  qDebug() << "[Component 3/5] Creating HotkeyManager...";
  m_hotkeyManager = new HotkeyManager(this);
  qDebug() << "[Component 3/5] HotkeyManager created successfully";

  // Register all hotkeys from configuration
  qDebug() << "[Component 3/5] Registering hotkeys from configuration...";
  registerAllHotkeys();
  qDebug() << "[Component 3/5] Hotkeys registered successfully";

  // Create main window
  qDebug() << "[Component 4/5] Creating MainWindow...";
  if (!m_clipboardManager) {
    qCritical() << "[Component 4/5] ERROR: Cannot create MainWindow - "
                   "ClipboardManager is null";
    throw std::runtime_error("ClipboardManager is null");
  }
  m_mainWindow = new MainWindow(m_clipboardManager, nullptr);
  qDebug() << "[Component 4/5] MainWindow created successfully";

  // Create tray icon
  qDebug() << "[Component 5/5] Creating TrayIcon...";
  if (!m_mainWindow) {
    qCritical()
        << "[Component 5/5] ERROR: Cannot create TrayIcon - MainWindow is null";
    throw std::runtime_error("MainWindow is null");
  }
  m_trayIcon = new TrayIcon(m_mainWindow, this);
  qDebug() << "[Component 5/5] TrayIcon created successfully";

  qDebug() << "[Component 5/5] Showing tray icon...";
  m_trayIcon->show();
  qDebug() << "[Component 5/5] Tray icon shown successfully";

  qDebug() << "All components initialized successfully";
  qDebug() << "Application::setupComponents() - EXIT";
}

void Application::connectSignals() {
  qDebug() << "Application::connectSignals() - ENTRY";
  qDebug() << "Connecting signal-slot connections...";

  // Hotkey activation - single connection with type parameter
  qDebug() << "Connecting HotkeyManager::hotkeyPressed to "
              "Application::onHotkeyPressed...";
  if (!m_hotkeyManager) {
    qCritical()
        << "ERROR: Cannot connect hotkey signal - m_hotkeyManager is null";
    throw std::runtime_error("HotkeyManager is null");
  }
  connect(m_hotkeyManager, &HotkeyManager::hotkeyPressed, this,
          &Application::onHotkeyPressed);
  qDebug() << "Hotkey signal connected successfully";

  // Network status changes
  qDebug() << "Connecting NetworkMonitor::onlineStatusChanged to "
              "Application::onNetworkStatusChanged...";
  if (!m_networkMonitor) {
    qCritical()
        << "ERROR: Cannot connect network signal - m_networkMonitor is null";
    throw std::runtime_error("NetworkMonitor is null");
  }
  connect(m_networkMonitor, &NetworkMonitor::onlineStatusChanged, this,
          &Application::onNetworkStatusChanged);
  qDebug() << "Network signal connected successfully";

  // Configuration change notifications
  qDebug() << "Connecting AppConfig::settingsChanged to "
              "Application::onSettingsChanged...";
  connect(AppConfig::instance(), &AppConfig::settingsChanged, this,
          &Application::onSettingsChanged, Qt::UniqueConnection);
  qDebug() << "Settings change signal connected successfully";

  // Tray icon actions
  if (!m_trayIcon || !m_mainWindow) {
    qCritical() << "ERROR: Cannot connect tray signals - TrayIcon or "
                   "MainWindow is null";
    throw std::runtime_error("TrayIcon or MainWindow is null");
  }

  qDebug() << "Connecting TrayIcon::showWindowRequested to "
              "MainWindow::showAndActivate...";
  connect(m_trayIcon, &TrayIcon::showWindowRequested, m_mainWindow,
          &MainWindow::showAndActivate);
  qDebug() << "Show window signal connected successfully";

  qDebug() << "Connecting TrayIcon::hideWindowRequested to MainWindow::hide...";
  connect(m_trayIcon, &TrayIcon::hideWindowRequested, m_mainWindow,
          &MainWindow::hide);
  qDebug() << "Hide window signal connected successfully";

  qDebug() << "Connecting TrayIcon::quitRequested to QApplication::quit...";
  connect(m_trayIcon, &TrayIcon::quitRequested, this, []() {
    qDebug() << "Quit requested via tray icon";
    QApplication::quit();
  });
  qDebug() << "Quit signal connected successfully";

  qDebug() << "All signal-slot connections established";
  qDebug() << "Application::connectSignals() - EXIT";
}

void Application::registerAllHotkeys() {
  qDebug() << "Application::registerAllHotkeys() - ENTRY";

  for (int i = 0; i < HotkeyType::Count; ++i) {
    HotkeyType::Type type = static_cast<HotkeyType::Type>(i);

    int key = AppConfig::instance()->getHotkeyKey(type);
    Qt::KeyboardModifiers modifiers =
        AppConfig::instance()->getHotkeyModifiers(type);

    bool registered = m_hotkeyManager->registerHotkey(type, key, modifiers);
    if (registered) {
      qDebug() << "Registered hotkey:" << HotkeyType::toDisplayName(type)
               << "as:" << QKeySequence(key | modifiers).toString();
    } else {
      qWarning() << "Failed to register hotkey:"
                 << HotkeyType::toDisplayName(type);
    }
  }

  qDebug() << "Application::registerAllHotkeys() - EXIT";
}

void Application::updateAllHotkeys() {
  qDebug() << "Application::updateAllHotkeys() - ENTRY";

  for (int i = 0; i < HotkeyType::Count; ++i) {
    HotkeyType::Type type = static_cast<HotkeyType::Type>(i);

    int key = AppConfig::instance()->getHotkeyKey(type);
    Qt::KeyboardModifiers modifiers =
        AppConfig::instance()->getHotkeyModifiers(type);

    m_hotkeyManager->updateHotkey(type, key, modifiers);

    qDebug() << "Updated hotkey:" << HotkeyType::toDisplayName(type)
             << "to:" << QKeySequence(key | modifiers).toString();
  }

  qDebug() << "Application::updateAllHotkeys() - EXIT";
}

void Application::onHotkeyPressed(int type) {
  qDebug() << "Application::onHotkeyPressed() - ENTRY";
  qDebug() << "Hotkey type:"
           << HotkeyType::toDisplayName(static_cast<HotkeyType::Type>(type));

  if (!m_mainWindow) {
    qWarning() << "Hotkey pressed but m_mainWindow is null - ignoring";
    return;
  }

  HotkeyType::Type hotkeyType = static_cast<HotkeyType::Type>(type);

  // Toggle behavior: if window is visible, hide it; otherwise show and run
  // script
  if (m_mainWindow->isVisible() && !m_mainWindow->isMinimized()) {
    qDebug() << "Window is visible - hiding";
    m_mainWindow->hide();
    qDebug() << "Application::onHotkeyPressed() - EXIT (hidden)";
    return;
  }

  // Window is hidden or minimized - show it
  qDebug() << "Window is hidden - showing and executing script";
  m_mainWindow->showAndActivate();

  // Handle Startup Behavior (e.g. switch to default tab if configured)
  if (ResourceManager::instance()->getStartupMode() ==
      ResourceManager::SelectedResource) {
    QString defaultId = ResourceManager::instance()->getDefaultResourceId();
    m_mainWindow->switchToResourceById(defaultId);
  }

  switch (hotkeyType) {
  case HotkeyType::MainToggle:
    // Execute Main Script
    qDebug() << "Executing Main Script";
    m_mainWindow->insertClipboardText(false /* Use Main Script */);
    break;

  case HotkeyType::AlternativeToggle:
    // Execute Alternative Script
    qDebug() << "Executing Alternative Script";
    m_mainWindow->insertClipboardText(true /* Use Alt Script */);
    break;

  default:
    qWarning() << "Unknown hotkey type:" << type;
    break;
  }

  qDebug() << "Application::onHotkeyPressed() - EXIT";
}

void Application::onNetworkStatusChanged(bool online) {
  qDebug() << "Application::onNetworkStatusChanged() - ENTRY";
  qDebug() << "Network status changed to:" << (online ? "ONLINE" : "OFFLINE");

  if (m_mainWindow) {
    qDebug() << "Updating MainWindow online status";
    m_mainWindow->setOnlineStatus(online);
    qDebug() << "MainWindow online status updated";
  } else {
    qWarning() << "Cannot update window status - m_mainWindow is null";
  }

  qInfo() << "Network status changed:" << (online ? "Online" : "Offline");
  qDebug() << "Application::onNetworkStatusChanged() - EXIT";
}

void Application::onSettingsChanged() {
  qDebug() << "Application::onSettingsChanged() - ENTRY";
  qDebug() << "Settings changed - updating components";

  try {
    // Apply WebView theme
    bool darkTheme = AppConfig::instance()->getDarkTheme();
    if (m_mainWindow) {
      qDebug() << "Updating WebView theme...";
      m_mainWindow->applyWebViewTheme(darkTheme);
      qDebug() << "WebView theme updated:" << (darkTheme ? "dark" : "light");
    } else {
      qWarning() << "Cannot update WebView theme - m_mainWindow is null";
    }

    // Update all hotkeys
    if (m_hotkeyManager) {
      qDebug() << "Updating all hotkeys...";
      updateAllHotkeys();
      qDebug() << "All hotkeys updated successfully";
    } else {
      qWarning() << "Cannot update hotkeys - m_hotkeyManager is null";
    }

    // Note: Other settings (window position, opacity, etc.) are applied by
    // MainWindow when they change, so we don't need to update them here

    qDebug() << "Settings applied successfully";
    qDebug() << "Application::onSettingsChanged() - EXIT";
  } catch (const std::exception &e) {
    qCritical() << "Application::onSettingsChanged() - EXCEPTION:" << e.what();
    qDebug() << "Application::onSettingsChanged() - EXIT with error";
  }
}
