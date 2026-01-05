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
}

Application::~Application() {
    // Cleanup handled by QPointer
}

void Application::initialize() {
    qDebug() << "Initializing Yandex Translator...";
    
    try {
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
    } catch (const std::exception& e) {
        qCritical() << "Exception during initialization:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception during initialization";
    }
}

void Application::setupComponents() {
    qDebug() << "Setting up components...";
    
    // Initialize clipboard manager
    qDebug() << "Creating ClipboardManager...";
    m_clipboardManager = new ClipboardManager(this);
    qDebug() << "ClipboardManager created";
    
    // Initialize network monitor
    qDebug() << "Creating NetworkMonitor...";
    m_networkMonitor = new NetworkMonitor(this);
    qDebug() << "NetworkMonitor created";
    
    // Initialize hotkey manager
    qDebug() << "Creating HotkeyManager...";
    m_hotkeyManager = new HotkeyManager(this);
    qDebug() << "HotkeyManager created";
    
    // Register default hotkey (Ctrl+Alt+T)
    qDebug() << "Registering hotkey...";
    if (!m_hotkeyManager->registerHotkey(Qt::Key_T, Qt::ControlModifier | Qt::AltModifier)) {
        qWarning() << "Failed to register default hotkey";
    }
    qDebug() << "Hotkey registered";
    
    // Create main window
    qDebug() << "Creating MainWindow...";
    m_mainWindow = new MainWindow(m_clipboardManager, nullptr);
    qDebug() << "MainWindow created";
    
    // Create tray icon
    qDebug() << "Creating TrayIcon...";
    m_trayIcon = new TrayIcon(m_mainWindow, this);
    qDebug() << "TrayIcon created";
    qDebug() << "Showing tray icon...";
    m_trayIcon->show();
    qDebug() << "Tray icon shown";
    
    qDebug() << "Components setup complete";
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
            this, []() { QApplication::quit(); });
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
