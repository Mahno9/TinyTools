#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QPointer>

class MainWindow;

class TrayIcon : public QObject {
    Q_OBJECT
    
public:
    explicit TrayIcon(MainWindow* mainWindow, QObject* parent = nullptr);
    ~TrayIcon();
    
    void show();
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
    void onReload();
    void onQuit();
    
private:
    void createContextMenu();
    void updateIconState();
    
    QSystemTrayIcon* m_trayIcon;
    QPointer<MainWindow> m_mainWindow;
    QIcon m_icon;
    QIcon m_iconActive;
};
