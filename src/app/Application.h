#pragma once
#include <QObject>
#include <QPointer>

#include "../core/HotkeyManager.h"

class MainWindow;
class TrayIcon;
class ClipboardManager;
class NetworkMonitor;

class Application : public QObject {
    Q_OBJECT
    
public:
    explicit Application(QObject* parent = nullptr);
    ~Application();
    
    void initialize();
    
private slots:
    void onHotkeyPressed(int type);
    void onResourceHotkeyPressed(const QString& resourceId, bool isAlt);
    void onNetworkStatusChanged(bool online);
    void onSettingsChanged();
    
    // Resource change handlers
    void onResourceAdded(const QString& resourceId);
    void onResourceRemoved(const QString& resourceId);
    void onResourceUpdated(const QString& resourceId);
    
private:
    void setupComponents();
    void connectSignals();
    void registerAllHotkeys();
    void updateAllHotkeys();
    
    void refreshResourceHotkeys(const QString& resourceId);
    void registerResourceHotkeys(); // Register all resources
    
    QPointer<MainWindow> m_mainWindow;
    QPointer<TrayIcon> m_trayIcon;
    QPointer<HotkeyManager> m_hotkeyManager;
    QPointer<ClipboardManager> m_clipboardManager;
    QPointer<NetworkMonitor> m_networkMonitor;
    
    Q_DISABLE_COPY(Application)
};
