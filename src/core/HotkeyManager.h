#pragma once
#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

class HotkeyManager : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
    
public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();
    
    bool registerHotkey(int key, Qt::KeyboardModifiers modifiers);
    bool unregisterHotkey();
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
protected:
    bool nativeEventFilter(const QByteArray& eventType, 
                          void* message, 
                          qintptr* result) override;
    
signals:
    void hotkeyPressed();
    
private:
    struct HotkeyData {
        int id;
        int key;
        Qt::KeyboardModifiers modifiers;
        bool registered;
    };
    
    HotkeyData m_hotkey;
    bool m_enabled;
    static int s_hotkeyIdCounter;
};
