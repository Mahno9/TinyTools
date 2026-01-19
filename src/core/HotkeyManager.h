#pragma once
#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <QMap>

namespace HotkeyType {
    enum Type {
        MainToggle = 0,
        AlternativeToggle,
        Count
    };
    
    inline const char* toString(Type type) {
        switch (type) {
            case MainToggle: return "mainToggleHotkey";
            case AlternativeToggle: return "alternativeToggleHotkey";
            default: return "unknown";
        }
    }
    
    inline const char* toDisplayName(Type type) {
        switch (type) {
            case MainToggle: return "Main Toggle";
            case AlternativeToggle: return "Alternative Toggle";
            default: return "Unknown";
        }
    }
}

class HotkeyManager : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
    
public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();
    
    // Generic hotkey management methods
    bool registerHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers);
    bool unregisterHotkey(HotkeyType::Type type);
    void updateHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers);
    void unregisterAll();
    
    // Resource hotkey management
    bool registerResourceHotkey(const QString& resourceId, bool isAlt, int key, Qt::KeyboardModifiers modifiers);
    void unregisterResourceHotkeys(const QString& resourceId);

    // Query methods
    bool isHotkeyRegistered(HotkeyType::Type type) const;
    int getHotkeyKey(HotkeyType::Type type) const;
    Qt::KeyboardModifiers getHotkeyModifiers(HotkeyType::Type type) const;
    
    // Enable/disable all hotkeys
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
protected:
    bool nativeEventFilter(const QByteArray& eventType, 
                          void* message, 
                          qintptr* result) override;
    
signals:
    void hotkeyPressed(HotkeyType::Type type);
    void resourceHotkeyPressed(const QString& resourceId, bool isAlt);
    
private:
    struct HotkeyData {
        int id;
        int key;
        Qt::KeyboardModifiers modifiers;
        bool registered;
        void* windowHandle;
    };
    
    struct ResourceHotkeyData {
        int id;
        QString resourceId;
        bool isAlt;
        int key;
        Qt::KeyboardModifiers modifiers;
        bool registered;
        void* windowHandle;
    };
    
    // Map-based storage for dynamic hotkey management
    QMap<HotkeyType::Type, HotkeyData> m_hotkeys;
    QMap<int, ResourceHotkeyData> m_resourceHotkeys; // Key: ID
    bool m_enabled;
    static int s_hotkeyIdCounter;
    
    // Helper methods
    void* getMainWindowHandle();
    bool unregisterHotkeyInternal(const HotkeyData& hotkey);
    HotkeyData* getHotkeyData(HotkeyType::Type type);
    const HotkeyData* getHotkeyData(HotkeyType::Type type) const;
    
    Q_DISABLE_COPY(HotkeyManager)
};
