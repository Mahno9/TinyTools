#include "HotkeyManager.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif

int HotkeyManager::s_hotkeyIdCounter = 0;

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
{
    m_hotkey.id = 0;
    m_hotkey.key = 0;
    m_hotkey.modifiers = Qt::NoModifier;
    m_hotkey.registered = false;
    
    // Install native event filter
    qApp->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager() {
    unregisterHotkey();
    qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::registerHotkey(int key, Qt::KeyboardModifiers modifiers) {
    // Unregister existing hotkey first
    if (m_hotkey.registered) {
        unregisterHotkey();
    }
    
#ifdef _WIN32
    UINT vkCode = key;
    UINT modifiersCode = 0;
    
    if (modifiers & Qt::ControlModifier) modifiersCode |= MOD_CONTROL;
    if (modifiers & Qt::AltModifier) modifiersCode |= MOD_ALT;
    if (modifiers & Qt::ShiftModifier) modifiersCode |= MOD_SHIFT;
    if (modifiers & Qt::MetaModifier) modifiersCode |= MOD_WIN;
    
    // Register hotkey with Windows
    m_hotkey.id = ++s_hotkeyIdCounter;
    
    // Get the main window handle
    HWND hwnd = nullptr;
    if (qApp && qApp->activeWindow()) {
        hwnd = (HWND)qApp->activeWindow()->winId();
    }
    
    BOOL result = RegisterHotKey(
        hwnd,
        m_hotkey.id,
        modifiersCode,
        vkCode
    );
    
    if (result) {
        m_hotkey.key = key;
        m_hotkey.modifiers = modifiers;
        m_hotkey.registered = true;
        qInfo() << "Registered hotkey:" << QKeySequence(key | modifiers).toString();
        return true;
    } else {
        qWarning() << "Failed to register hotkey:" << GetLastError();
        return false;
    }
#else
    // Linux implementation using QxtGlobalShortcut
    Q_UNUSED(key);
    Q_UNUSED(modifiers);
    return false;
#endif
}

bool HotkeyManager::unregisterHotkey() {
    if (!m_hotkey.registered) return true;
    
#ifdef _WIN32
    HWND hwnd = nullptr;
    if (qApp && qApp->activeWindow()) {
        hwnd = (HWND)qApp->activeWindow()->winId();
    }
    
    BOOL result = UnregisterHotKey(hwnd, m_hotkey.id);
    if (result) {
        m_hotkey.registered = false;
        qInfo() << "Unregistered hotkey";
        return true;
    } else {
        qWarning() << "Failed to unregister hotkey:" << GetLastError();
        return false;
    }
#else
    return false;
#endif
}

void HotkeyManager::setEnabled(bool enabled) {
    m_enabled = enabled;
}

bool HotkeyManager::nativeEventFilter(const QByteArray& eventType,
                                       void* message,
                                       qintptr* result) {
#ifdef _WIN32
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_HOTKEY && m_enabled && m_hotkey.registered) {
            if (msg->wParam == m_hotkey.id) {
                emit hotkeyPressed();
                return true;
            }
        }
    }
#endif
    
    return false;
}
