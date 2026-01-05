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
