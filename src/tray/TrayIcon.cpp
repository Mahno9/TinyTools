#include "TrayIcon.h"
#include "../ui/MainWindow.h"
#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QMenu>

TrayIcon::TrayIcon(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_mainWindow(mainWindow)
{
    m_trayIcon->setIcon(QIcon(":/icons/icon.png"));
    m_trayIcon->setToolTip("TinyTools");

    createContextMenu();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &TrayIcon::onActivated);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray is not available on this system";
    }
}

TrayIcon::~TrayIcon() {
    m_trayIcon->hide();
    delete m_menu;
}

void TrayIcon::show() {
    if (!m_trayIcon->isVisible()) {
        m_trayIcon->show();
    }
}

void TrayIcon::createContextMenu() {
    QMenu* menu = m_menu = new QMenu();

    QAction* showAction = menu->addAction("Show Window");
    connect(showAction, &QAction::triggered, this, &TrayIcon::onShowWindow);

    QAction* hideAction = menu->addAction("Hide Window");
    connect(hideAction, &QAction::triggered, this, &TrayIcon::onHideWindow);

    menu->addSeparator();

    QAction* toggleTopAction = menu->addAction("Toggle Always on Top");
    connect(toggleTopAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow) m_mainWindow->toggleAlwaysOnTop();
    });

    menu->addSeparator();

    QAction* settingsAction = menu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow) m_mainWindow->onSettingsRequested();
    });

    QAction* reloadAction = menu->addAction("Reload");
    connect(reloadAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow) m_mainWindow->reloadCurrentResource();
    });

    menu->addSeparator();

    QAction* quitAction = menu->addAction("Exit");
    connect(quitAction, &QAction::triggered, this, [this]() {
        emit quitRequested();
        QCoreApplication::quit();
    });

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
            onShowWindow();
            break;

        default:
            break;
    }
}

void TrayIcon::onShowWindow() {
    if (!m_mainWindow) {
        return;
    }
    m_mainWindow->showAndActivate();
    emit showWindowRequested();
}

void TrayIcon::onHideWindow() {
    if (!m_mainWindow) {
        return;
    }
    m_mainWindow->hide();
    emit hideWindowRequested();
}

void TrayIcon::showNotification(const QString& title, const QString& message) {
    m_trayIcon->showMessage(title, message, QSystemTrayIcon::Warning, 5000);
}
