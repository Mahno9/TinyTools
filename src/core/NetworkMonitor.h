#pragma once
#include <QObject>

class NetworkMonitor : public QObject {
    Q_OBJECT
    
public:
    explicit NetworkMonitor(QObject* parent = nullptr);
    ~NetworkMonitor();
    
    bool isOnline() const;
    
    void startMonitoring();
    void stopMonitoring();
    
signals:
    void onlineStatusChanged(bool online);
    
private:
    bool m_isOnline;
};
