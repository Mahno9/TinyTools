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
        qDebug() << "Step 1: Setting up components...";
        setupComponents();
        qDebug() << "Step 1 complete: Components setup finished";
        
        qDebug() << "Step 2: Connecting signals...";
        connectSignals();
        qDebug() << "Step 2 complete: Signals connected";
        
        qDebug() << "Step 3: Loading configuration and applying auto-start settings...";
        AppConfig config;
        if (config.load()) {
            qDebug() << "Configuration loaded successfully";
            qDebug() << "Auto-start setting:" << (config.getAutoStart() ? "enabled" : "disabled");
            qDebug() << "Window size:" << config.getWindowWidth() << "x" << config.getWindowHeight();
            qDebug() << "Window position:" << config.getWindowX() << "," << config.getWindowY();
            qDebug() << "Always on top:" << (config.getAlwaysOnTop() ? "yes" : "no");
            qDebug() << "Window opacity:" << config.getWindowOpacity() << "%";
            
            if (!config.getAutoStart()) {
                // Start hidden if auto-start is disabled
                qDebug() << "Auto-start disabled - hiding main window";
                if (m_mainWindow) {
                    m_mainWindow->hide();
                    qDebug() << "Main window hidden successfully";
                } else {
                    qWarning() << "Cannot hide window - m_mainWindow is null";
                }
            } else {
                qDebug() << "Auto-start enabled - showing main window";
                if (m_mainWindow) {
                    m_mainWindow->show();
                    qDebug() << "Main window shown successfully";
                } else {
                    qWarning() << "Cannot show window - m_mainWindow is null";
                }
            }
        } else {
            qWarning() << "Failed to load configuration - using defaults";
        }
        qDebug() << "Step 3 complete: Configuration applied";
        
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
    
    // Register default hotkey (Ctrl+Alt+T)
    qDebug() << "[Component 3/5] Registering default hotkey (Ctrl+Alt+T)...";
    bool hotkeyRegistered = m_hotkeyManager->registerHotkey(Qt::Key_T, Qt::ControlModifier | Qt::AltModifier);
    if (hotkeyRegistered) {
        qDebug() << "[Component 3/5] Default hotkey registered successfully";
    } else {
        qWarning() << "[Component 3/5] Failed to register default hotkey";
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
    
    // Network status changes
    qDebug() << "Connecting NetworkMonitor::onlineStatusChanged to Application::onNetworkStatusChanged...";
    if (!m_networkMonitor) {
        qCritical() << "ERROR: Cannot connect network signal - m_networkMonitor is null";
        throw std::runtime_error("NetworkMonitor is null");
    }
    connect(m_networkMonitor, &NetworkMonitor::onlineStatusChanged,
            this, &Application::onNetworkStatusChanged);
    qDebug() << "Network signal connected successfully";
    
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
    
    if (m_mainWindow->isVisible()) {
        qDebug() << "Hotkey pressed - window is visible, hiding window";
        m_mainWindow->hide();
    } else {
        qDebug() << "Hotkey pressed - window is hidden, showing window and inserting clipboard text";
        m_mainWindow->showAndActivate();
        m_mainWindow->insertClipboardText();
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
