#include "TrayIcon.h"
#include "../ui/MainWindow.h"
#include "../models/AppConfig.h"
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>

TrayIcon::TrayIcon(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_trayIcon(new QSystemTrayIcon(this))
{
    qDebug() << "TrayIcon::TrayIcon() - ENTRY";
    qDebug() << "Creating TrayIcon with mainWindow:" << (mainWindow ? "yes" : "no");
    qDebug() << "Parent object:" << (parent ? "yes" : "no");
    
    qDebug() << "Loading application icons...";
    // Load icons from resources
    m_icon = QIcon(":/icons/icon.png");
    m_iconActive = QIcon(":/icons/icon.png");
    qDebug() << "Icons loaded successfully";
    
    qDebug() << "Setting tray icon properties...";
    m_trayIcon->setIcon(m_icon);
    qDebug() << "Icon set to SP_ComputerIcon";
    
    m_trayIcon->setToolTip("TinyTools");
    qDebug() << "Tooltip set to: 'TinyTools'";
    
    qDebug() << "Creating context menu...";
    createContextMenu();
    qDebug() << "Context menu created";
    
    qDebug() << "Connecting tray activation signal...";
    // Connect activation signal
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &TrayIcon::onActivated);
    qDebug() << "Activation signal connected";
    
    qDebug() << "TrayIcon constructed successfully";
    qDebug() << "TrayIcon::TrayIcon() - EXIT";
}

TrayIcon::~TrayIcon() {
    qDebug() << "TrayIcon::~TrayIcon() - ENTRY";
    qDebug() << "Destroying TrayIcon";
    
    qDebug() << "Hiding tray icon...";
    m_trayIcon->hide();
    qDebug() << "Tray icon hidden";
    
    qDebug() << "TrayIcon destroyed";
    qDebug() << "TrayIcon::~TrayIcon() - EXIT";
}

void TrayIcon::show() {
    qDebug() << "TrayIcon::show() - ENTRY";
    qDebug() << "Showing tray icon in system tray...";
    
    if (!m_trayIcon->isVisible()) {
        m_trayIcon->show();
        qDebug() << "Tray icon is now visible";
    } else {
        qDebug() << "Tray icon is already visible";
    }
    
    qDebug() << "TrayIcon::show() - EXIT";
}

void TrayIcon::createContextMenu() {
    qDebug() << "TrayIcon::createContextMenu() - ENTRY";
    qDebug() << "Creating tray icon context menu...";
    
    QMenu* menu = new QMenu();
    qDebug() << "Menu object created";
    
    // Show/Hide Window
    qDebug() << "Adding 'Show Window' action...";
    QAction* showAction = menu->addAction("Show Window");
    connect(showAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow) {
            qDebug() << "Show Window action triggered - showing and activating window";
            m_mainWindow->showAndActivate();
        }
    });
    qDebug() << "'Show Window' action added";
    
    qDebug() << "Adding 'Hide Window' action...";
    QAction* hideAction = menu->addAction("Hide Window");
    connect(hideAction, &QAction::triggered, this, &TrayIcon::onHideWindow);
    qDebug() << "'Hide Window' action added";
    
    menu->addSeparator();
    qDebug() << "Separator added";
    
    // Toggle Always on Top
    qDebug() << "Adding 'Toggle Always on Top' action...";
    QAction* toggleTopAction = menu->addAction("Toggle Always on Top");
    connect(toggleTopAction, &QAction::triggered,
            this, &TrayIcon::onToggleAlwaysOnTop);
    qDebug() << "'Toggle Always on Top' action added";
    
    menu->addSeparator();
    qDebug() << "Separator added";
    
    // Settings
    qDebug() << "Adding 'Settings...' action...";
    QAction* settingsAction = menu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, &TrayIcon::onOpenSettings);
    qDebug() << "'Settings...' action added";
    
    // Reload
    qDebug() << "Adding 'Reload' action...";
    QAction* reloadAction = menu->addAction("Reload");
    connect(reloadAction, &QAction::triggered, this, &TrayIcon::onReload);
    qDebug() << "'Reload' action added";
    
    menu->addSeparator();
    qDebug() << "Separator added";
    
    // Quit
    qDebug() << "Adding 'Exit' action...";
    QAction* quitAction = menu->addAction("Exit");
    connect(quitAction, &QAction::triggered, this, &TrayIcon::onQuit);
    qDebug() << "'Exit' action added";
    
    qDebug() << "Setting context menu to tray icon...";
    m_trayIcon->setContextMenu(menu);
    qDebug() << "Context menu assigned to tray icon";
    
    qDebug() << "TrayIcon::createContextMenu() - EXIT";
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
    qDebug() << "TrayIcon::onActivated() - ENTRY";
    qDebug() << "Activation reason:" << reason;
    
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            qDebug() << "Single click detected - toggling window visibility";
            // Single click: toggle window visibility
            if (m_mainWindow && m_mainWindow->isVisible()) {
                qDebug() << "Window is visible - hiding it";
                onHideWindow();
            } else {
                qDebug() << "Window is hidden - showing it";
                onShowWindow();
            }
            break;
            
        case QSystemTrayIcon::DoubleClick:
            qDebug() << "Double click detected - showing window and inserting clipboard text";
            // Double click: show window and insert clipboard text
            onShowWindow();
            break;
            
        default:
            qDebug() << "Unhandled activation reason:" << reason;
            break;
    }
    
    qDebug() << "TrayIcon::onActivated() - EXIT";
}

void TrayIcon::onShowWindow() {
    qDebug() << "TrayIcon::onShowWindow() - ENTRY";
    
    if (m_mainWindow) {
        qDebug() << "Showing main window...";
        m_mainWindow->showAndActivate();
        qDebug() << "Main window shown and activated";
        
        // Check if auto-translate on clipboard is enabled
        bool autoTranslate = AppConfig::instance()->getAutoTranslate();
        qDebug() << "Auto-translate on clipboard setting:" << (autoTranslate ? "enabled" : "disabled");
        
        if (autoTranslate) {
            qDebug() << "Auto-translate enabled - inserting clipboard text...";
            m_mainWindow->insertClipboardText();
            qDebug() << "Clipboard text inserted";
        } else {
            qDebug() << "Auto-translate disabled - not inserting clipboard text";
        }
        
        qDebug() << "Emitting showWindowRequested signal...";
        emit showWindowRequested();
        qDebug() << "Signal emitted";
    } else {
        qWarning() << "Cannot show window - m_mainWindow is null";
    }
    
    qDebug() << "TrayIcon::onShowWindow() - EXIT";
}

void TrayIcon::onHideWindow() {
    qDebug() << "TrayIcon::onHideWindow() - ENTRY";
    
    if (m_mainWindow) {
        qDebug() << "Hiding main window...";
        m_mainWindow->hide();
        qDebug() << "Main window hidden";
        qDebug() << "Emitting hideWindowRequested signal...";
        emit hideWindowRequested();
        qDebug() << "Signal emitted";
    } else {
        qWarning() << "Cannot hide window - m_mainWindow is null";
    }
    
    qDebug() << "TrayIcon::onHideWindow() - EXIT";
}

void TrayIcon::onToggleAlwaysOnTop() {
    qDebug() << "TrayIcon::onToggleAlwaysOnTop() - ENTRY";
    
    if (m_mainWindow) {
        qDebug() << "Toggling always-on-top setting...";
        m_mainWindow->toggleAlwaysOnTop();
        qDebug() << "Always-on-top toggled";
    } else {
        qWarning() << "Cannot toggle always-on-top - m_mainWindow is null";
    }
    
    qDebug() << "TrayIcon::onToggleAlwaysOnTop() - EXIT";
}

void TrayIcon::onOpenSettings() {
    qDebug() << "TrayIcon::onOpenSettings() - ENTRY";
    
    if (m_mainWindow) {
        qDebug() << "Opening settings dialog via MainWindow...";
        m_mainWindow->onSettingsRequested();
        qInfo() << "Settings dialog opened";
    } else {
        qWarning() << "Cannot open settings - m_mainWindow is null";
    }
    
    qDebug() << "TrayIcon::onOpenSettings() - EXIT";
}

void TrayIcon::onReload() {
    qDebug() << "TrayIcon::onReload() - ENTRY";

    if (m_mainWindow) {
        qDebug() << "Reloading via MainWindow...";
        m_mainWindow->setOnlineStatus(true);
        qInfo() << "Reloaded";
    } else {
        qWarning() << "Cannot reload - m_mainWindow is null";
    }

    qDebug() << "TrayIcon::onReload() - EXIT";
}

void TrayIcon::onQuit() {
    qDebug() << "TrayIcon::onQuit() - ENTRY";
    qDebug() << "Quit requested from tray icon";
    qDebug() << "Emitting quitRequested signal...";
    emit quitRequested();
    qDebug() << "Signal emitted";
    qDebug() << "Quitting application...";
    QCoreApplication::quit();
    qDebug() << "TrayIcon::onQuit() - EXIT";
}

void TrayIcon::showNotification(const QString& title, const QString& message) {
    qDebug() << "TrayIcon::showNotification() - ENTRY";
    qDebug() << "Notification title:" << title;
    qDebug() << "Notification message:" << message;
    
    qDebug() << "Showing notification...";
    m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    qDebug() << "Notification displayed";
    
    qDebug() << "TrayIcon::showNotification() - EXIT";
}

void TrayIcon::updateIconState() {
    qDebug() << "TrayIcon::updateIconState() - ENTRY";
    
    if (m_mainWindow && m_mainWindow->isVisible()) {
        qDebug() << "Window is visible - switching to active icon";
        m_trayIcon->setIcon(m_iconActive);
        qDebug() << "Active icon set";
    } else {
        qDebug() << "Window is hidden - switching to normal icon";
        m_trayIcon->setIcon(m_icon);
        qDebug() << "Normal icon set";
    }
    
    qDebug() << "TrayIcon::updateIconState() - EXIT";
}
