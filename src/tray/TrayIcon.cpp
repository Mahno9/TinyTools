#include "TrayIcon.h"
#include "../ui/MainWindow.h"
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QCoreApplication>
#include <QDebug>

TrayIcon::TrayIcon(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_trayIcon(new QSystemTrayIcon(this))
{
    // Load icons (using style icon since actual icon files don't exist)
    m_icon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
    m_iconActive = QApplication::style()->standardIcon(QStyle::SP_DialogYesButton);
    
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
