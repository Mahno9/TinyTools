#include "Application.h"
#include "../ui/MainWindow.h"
#include "../tray/TrayIcon.h"
#include "../core/HotkeyManager.h"
#include "../core/ClipboardManager.h"
#include "../core/NetworkMonitor.h"
#include "../models/AppConfig.h"
#include <QApplication>
#include <QDebug>

Application::Application(QObject* parent)
    : QObject(parent)
{
    qDebug() << "Application::Application() - ENTRY";
    qDebug() << "Application instance constructed with parent:" << (parent ? "yes" : "no");
    qDebug() << "Application::Application() - EXIT";
}

Application::~Application() {
    qDebug() << "Application::~Application() - ENTRY";
    qDebug() << "Application instance destroyed - cleanup handled by QPointer";
    qDebug() << "Application::~Application() - EXIT";
}

void Application::initialize() {
    qDebug() << "Application::initialize() - ENTRY";
    qDebug() << "Initializing Yandex Translator application components...";
    
    try {
        qDebug() << "Step 1: Loading configuration...";
        if (!AppConfig::instance()->load()) {
            qWarning() << "Failed to load configuration - using defaults";
            qDebug() << "Using default configuration values";
        } else {
            qDebug() << "Configuration loaded successfully";
            qDebug() << "Show window on startup:" << (AppConfig::instance()->getShowWindowOnStartup() ? "enabled" : "disabled");
            qDebug() << "Auto-start on login:" << (AppConfig::instance()->getAutoStartOnLogin() ? "enabled" : "disabled");
            qDebug() << "Window size:" << AppConfig::instance()->getWindowWidth() << "x" << AppConfig::instance()->getWindowHeight();
            qDebug() << "Window position:" << AppConfig::instance()->getWindowX() << "," << AppConfig::instance()->getWindowY();
            qDebug() << "Always on top:" << (AppConfig::instance()->getAlwaysOnTop() ? "yes" : "no");
            qDebug() << "Window opacity:" << AppConfig::instance()->getWindowOpacity() << "%";
            qDebug() << "Hotkey key:" << AppConfig::instance()->getHotkeyKey();
            qDebug() << "Hotkey modifiers:" << QKeySequence(AppConfig::instance()->getHotkeyKey() | AppConfig::instance()->getHotkeyModifiers()).toString();
        }
        qDebug() << "Step 1 complete: Configuration loaded";
        
        qDebug() << "Step 2: Setting up components...";
        setupComponents();
        qDebug() << "Step 2 complete: Components setup finished";
        
        qDebug() << "Step 3: Connecting signals...";
        connectSignals();
        qDebug() << "Step 3 complete: Signals connected";
        
        qDebug() << "Step 4: Applying startup settings...";
        bool showWindow = AppConfig::instance()->getShowWindowOnStartup();
        bool autoStartOnLogin = AppConfig::instance()->isAutoStartEnabledInRegistry();
        
        qDebug() << "Show window on startup:" << (showWindow ? "YES" : "NO");
        qDebug() << "Auto-start on login (registry):" << (autoStartOnLogin ? "YES" : "NO");
        
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
    } catch (const std::exception& e) {
        qCritical() << "Application::initialize() - EXCEPTION: Standard exception:" << e.what();
        qCritical() << "Application::initialize() - EXIT with error";
        throw;
    } catch (...) {
        qCritical() << "Application::initialize() - EXCEPTION: Unknown exception caught";
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
    
    // Register hotkey from configuration (will use defaults if config not loaded yet)
    qDebug() << "[Component 3/5] Registering hotkey from configuration...";
    int hotkeyKey = AppConfig::instance()->getHotkeyKey();
    Qt::KeyboardModifiers hotkeyModifiers = AppConfig::instance()->getHotkeyModifiers();
    bool hotkeyRegistered = m_hotkeyManager->registerHotkey(hotkeyKey, hotkeyModifiers);
    if (hotkeyRegistered) {
        qDebug() << "[Component 3/5] Hotkey registered successfully:" << QKeySequence(hotkeyKey | hotkeyModifiers).toString();
    } else {
        qWarning() << "[Component 3/5] Failed to register hotkey";
    }
    
    // Register show and translate hotkey from configuration
    qDebug() << "[Component 3.5] Registering show and translate hotkey from configuration...";
    int showTranslateKey = AppConfig::instance()->getShowTranslateKey();
    Qt::KeyboardModifiers showTranslateModifiers = AppConfig::instance()->getShowTranslateModifiers();
    bool showTranslateRegistered = m_hotkeyManager->registerShowTranslateHotkey(showTranslateKey, showTranslateModifiers);
    if (showTranslateRegistered) {
        qDebug() << "[Component 3.5] Show and translate hotkey registered successfully:" << QKeySequence(showTranslateKey | showTranslateModifiers).toString();
    } else {
        qWarning() << "[Component 3.5] Failed to register show and translate hotkey";
    }
    
    // Create main window
    qDebug() << "[Component 4/5] Creating MainWindow...";
    if (!m_clipboardManager) {
        qCritical() << "[Component 4/5] ERROR: Cannot create MainWindow - ClipboardManager is null";
        throw std::runtime_error("ClipboardManager is null");
    }
    m_mainWindow = new MainWindow(m_clipboardManager, nullptr);
    qDebug() << "[Component 4/5] MainWindow created successfully";
    
    // Create tray icon
    qDebug() << "[Component 5/5] Creating TrayIcon...";
    if (!m_mainWindow) {
        qCritical() << "[Component 5/5] ERROR: Cannot create TrayIcon - MainWindow is null";
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
    
    // Hotkey activation
    qDebug() << "Connecting HotkeyManager::hotkeyPressed to Application::onHotkeyPressed...";
    if (!m_hotkeyManager) {
        qCritical() << "ERROR: Cannot connect hotkey signal - m_hotkeyManager is null";
        throw std::runtime_error("HotkeyManager is null");
    }
    connect(m_hotkeyManager, &HotkeyManager::hotkeyPressed,
            this, &Application::onHotkeyPressed);
    qDebug() << "Hotkey signal connected successfully";
    
    // Show and Translate hotkey activation
    qDebug() << "Connecting HotkeyManager::showTranslateHotkeyPressed to Application::onShowTranslateHotkeyPressed...";
    if (!m_hotkeyManager) {
        qCritical() << "ERROR: Cannot connect show translate hotkey signal - m_hotkeyManager is null";
        throw std::runtime_error("HotkeyManager is null");
    }
    connect(m_hotkeyManager, &HotkeyManager::showTranslateHotkeyPressed,
            this, &Application::onShowTranslateHotkeyPressed);
    qDebug() << "Show and Translate hotkey signal connected successfully";
    
    // Network status changes
    qDebug() << "Connecting NetworkMonitor::onlineStatusChanged to Application::onNetworkStatusChanged...";
    if (!m_networkMonitor) {
        qCritical() << "ERROR: Cannot connect network signal - m_networkMonitor is null";
        throw std::runtime_error("NetworkMonitor is null");
    }
    connect(m_networkMonitor, &NetworkMonitor::onlineStatusChanged,
            this, &Application::onNetworkStatusChanged);
    qDebug() << "Network signal connected successfully";
    
    // Configuration change notifications
    qDebug() << "Connecting AppConfig::settingsChanged to Application::onSettingsChanged...";
    connect(AppConfig::instance(), &AppConfig::settingsChanged,
            this, &Application::onSettingsChanged, Qt::UniqueConnection);
    qDebug() << "Settings change signal connected successfully";
    
    // Tray icon actions
    if (!m_trayIcon || !m_mainWindow) {
        qCritical() << "ERROR: Cannot connect tray signals - TrayIcon or MainWindow is null";
        throw std::runtime_error("TrayIcon or MainWindow is null");
    }
    
    qDebug() << "Connecting TrayIcon::showWindowRequested to MainWindow::showAndActivate...";
    connect(m_trayIcon, &TrayIcon::showWindowRequested,
            m_mainWindow, &MainWindow::showAndActivate);
    qDebug() << "Show window signal connected successfully";
    
    qDebug() << "Connecting TrayIcon::hideWindowRequested to MainWindow::hide...";
    connect(m_trayIcon, &TrayIcon::hideWindowRequested,
            m_mainWindow, &MainWindow::hide);
    qDebug() << "Hide window signal connected successfully";
    
    qDebug() << "Connecting TrayIcon::quitRequested to QApplication::quit...";
    connect(m_trayIcon, &TrayIcon::quitRequested,
            this, []() {
                qDebug() << "Quit requested via tray icon";
                QApplication::quit();
            });
    qDebug() << "Quit signal connected successfully";
    
    qDebug() << "All signal-slot connections established";
    qDebug() << "Application::connectSignals() - EXIT";
}

void Application::onHotkeyPressed() {
    qDebug() << "Application::onHotkeyPressed() - ENTRY";
    
    if (!m_mainWindow) {
        qWarning() << "Hotkey pressed but m_mainWindow is null - ignoring";
        qDebug() << "Application::onHotkeyPressed() - EXIT";
        return;
    }
    
    // Check if auto-translate on clipboard is enabled
    bool autoTranslate = AppConfig::instance()->getAutoTranslate();
    qDebug() << "Auto-translate on clipboard setting:" << (autoTranslate ? "enabled" : "disabled");
    
    if (m_mainWindow->isVisible()) {
        qDebug() << "Hotkey pressed - window is visible, hiding window";
        m_mainWindow->hide();
    } else {
        qDebug() << "Hotkey pressed - window is hidden, showing window";
        m_mainWindow->showAndActivate();
        
        // Only insert clipboard text if auto-translate is enabled
        if (autoTranslate) {
            qDebug() << "Auto-translate enabled - inserting clipboard text";
            m_mainWindow->insertClipboardText();
        } else {
            qDebug() << "Auto-translate disabled - not inserting clipboard text";
        }
    }
    
    qDebug() << "Application::onHotkeyPressed() - EXIT";
}

void Application::onShowTranslateHotkeyPressed() {
    qDebug() << "Application::onShowTranslateHotkeyPressed() - ENTRY";
    
    if (!m_mainWindow) {
        qWarning() << "Show and Translate hotkey pressed but m_mainWindow is null - ignoring";
        qDebug() << "Application::onShowTranslateHotkeyPressed() - EXIT";
        return;
    }
    
    // Show window and insert clipboard text (always, independent of auto-translate setting)
    qDebug() << "Show and Translate hotkey pressed - showing window and inserting clipboard text";
    m_mainWindow->showAndActivate();
    m_mainWindow->insertClipboardText();
    
    qDebug() << "Application::onShowTranslateHotkeyPressed() - EXIT";
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
        
        // Update hotkey if changed
        if (m_hotkeyManager) {
            qDebug() << "Updating hotkey...";
            int hotkeyKey = AppConfig::instance()->getHotkeyKey();
            Qt::KeyboardModifiers hotkeyModifiers = AppConfig::instance()->getHotkeyModifiers();
            m_hotkeyManager->updateHotkey(hotkeyKey, hotkeyModifiers);
            qDebug() << "Hotkey updated to:" << QKeySequence(hotkeyKey | hotkeyModifiers).toString();
        } else {
            qWarning() << "Cannot update hotkey - m_hotkeyManager is null";
        }
        
        // Update show and translate hotkey if changed
        if (m_hotkeyManager) {
            qDebug() << "Updating show and translate hotkey...";
            int showTranslateKey = AppConfig::instance()->getShowTranslateKey();
            Qt::KeyboardModifiers showTranslateModifiers = AppConfig::instance()->getShowTranslateModifiers();
            m_hotkeyManager->updateShowTranslateHotkey(showTranslateKey, showTranslateModifiers);
            qDebug() << "Show and Translate hotkey updated to:" << QKeySequence(showTranslateKey | showTranslateModifiers).toString();
        } else {
            qWarning() << "Cannot update show and translate hotkey - m_hotkeyManager is null";
        }
        
        // Note: Other settings (window position, opacity, etc.) are applied by MainWindow
        // when they change, so we don't need to update them here
        
        qDebug() << "Settings applied successfully";
        qDebug() << "Application::onSettingsChanged() - EXIT";
    } catch (const std::exception& e) {
        qCritical() << "Application::onSettingsChanged() - EXCEPTION:" << e.what();
        qDebug() << "Application::onSettingsChanged() - EXIT with error";
    }
}
