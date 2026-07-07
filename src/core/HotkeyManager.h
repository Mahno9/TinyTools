#pragma once
#include <QAbstractNativeEventFilter>
#include <QKeySequence>
#include <QMap>
#include <QObject>

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

    bool registerHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers);
    bool unregisterHotkey(HotkeyType::Type type);
    // Re-registers only if key/modifiers actually changed. Returns false when
    // a change was attempted and registration failed.
    bool updateHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers);
    void unregisterAll();

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

private:
    struct HotkeyData {
        int id;
        int key;
        Qt::KeyboardModifiers modifiers;
        bool registered;
    };

    QMap<HotkeyType::Type, HotkeyData> m_hotkeys;
    bool m_enabled;
    static int s_hotkeyIdCounter;

    bool unregisterHotkeyInternal(const HotkeyData& hotkey);
    HotkeyData* getHotkeyData(HotkeyType::Type type);
    const HotkeyData* getHotkeyData(HotkeyType::Type type) const;

    Q_DISABLE_COPY(HotkeyManager)
};
