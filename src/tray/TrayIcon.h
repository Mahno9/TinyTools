#pragma once
#include <QObject>
#include <QPointer>
#include <QSystemTrayIcon>

class MainWindow;
class QMenu;

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

private:
    void createContextMenu();

    QSystemTrayIcon* m_trayIcon;
    QMenu* m_menu = nullptr; // QSystemTrayIcon does not take ownership
    QPointer<MainWindow> m_mainWindow;
};
