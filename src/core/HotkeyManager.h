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
    bool registerShowTranslateHotkey(int key, Qt::KeyboardModifiers modifiers);
    bool unregisterHotkey();
    void updateHotkey(int keyCode, Qt::KeyboardModifiers modifiers);
    void updateShowTranslateHotkey(int keyCode, Qt::KeyboardModifiers modifiers);
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
protected:
    bool nativeEventFilter(const QByteArray& eventType, 
                          void* message, 
                          qintptr* result) override;
    
signals:
    void hotkeyPressed();
    void showTranslateHotkeyPressed();
    
private:
    struct HotkeyData {
        int id;
        int key;
        Qt::KeyboardModifiers modifiers;
        bool registered;
        void* windowHandle;  // Store HWND used for registration
    };
    
    HotkeyData m_hotkey;
    HotkeyData m_showTranslateHotkey;
    bool m_enabled;
    static int s_hotkeyIdCounter;
    
    // Helper methods for consistent window handle handling
    void* getMainWindowHandle();
    bool unregisterHotkeyWithHandle(void* hwnd, int hotkeyId);
};
