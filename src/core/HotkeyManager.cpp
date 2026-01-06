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
    qDebug() << "HotkeyManager::HotkeyManager() - ENTRY";
    qDebug() << "Creating HotkeyManager with parent:" << (parent ? "yes" : "no");
    
    m_hotkey.id = 0;
    m_hotkey.key = 0;
    m_hotkey.modifiers = Qt::NoModifier;
    m_hotkey.registered = false;
    qDebug() << "Hotkey data initialized";
    qDebug() << "Hotkey enabled:" << (m_enabled ? "yes" : "no");
    
    qDebug() << "Installing native event filter...";
    qApp->installNativeEventFilter(this);
    qDebug() << "Native event filter installed successfully";
    
    qDebug() << "HotkeyManager initialized and ready to register hotkeys";
    qDebug() << "HotkeyManager::HotkeyManager() - EXIT";
}

HotkeyManager::~HotkeyManager() {
    qDebug() << "HotkeyManager::~HotkeyManager() - ENTRY";
    qDebug() << "Destroying HotkeyManager";
    
    qDebug() << "Unregistering any registered hotkey...";
    unregisterHotkey();
    qDebug() << "Hotkey unregistered";
    
    qDebug() << "Removing native event filter...";
    qApp->removeNativeEventFilter(this);
    qDebug() << "Native event filter removed";
    
    qDebug() << "HotkeyManager destroyed";
    qDebug() << "HotkeyManager::~HotkeyManager() - EXIT";
}

bool HotkeyManager::registerHotkey(int key, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::registerHotkey() - ENTRY";
    qDebug() << "Key:" << key;
    qDebug() << "Modifiers:" << QKeySequence(key | modifiers).toString();
    
    // Unregister existing hotkey first
    if (m_hotkey.registered) {
        qDebug() << "Unregistering existing hotkey before registering new one...";
        unregisterHotkey();
        qDebug() << "Existing hotkey unregistered";
    }
    
#ifdef _WIN32
    qDebug() << "Windows platform detected - using Windows hotkey API";
    
    UINT vkCode = key;
    UINT modifiersCode = 0;
    
    qDebug() << "Parsing modifiers...";
    if (modifiers & Qt::ControlModifier) {
        modifiersCode |= MOD_CONTROL;
        qDebug() << "  - Control";
    }
    if (modifiers & Qt::AltModifier) {
        modifiersCode |= MOD_ALT;
        qDebug() << "  - Alt";
    }
    if (modifiers & Qt::ShiftModifier) {
        modifiersCode |= MOD_SHIFT;
        qDebug() << "  - Shift";
    }
    if (modifiers & Qt::MetaModifier) {
        modifiersCode |= MOD_WIN;
        qDebug() << "  - Meta (Win)";
    }
    
    // Register hotkey with Windows
    m_hotkey.id = ++s_hotkeyIdCounter;
    qDebug() << "New hotkey ID:" << m_hotkey.id;
    
    // Get the main window handle
    HWND hwnd = nullptr;
    qDebug() << "Getting main window handle for hotkey registration...";
    if (qApp && qApp->activeWindow()) {
        hwnd = (HWND)qApp->activeWindow()->winId();
        qDebug() << "Window handle obtained:" << (void*)hwnd;
    } else {
        qWarning() << "No active window found - using null window handle";
    }
    
    qDebug() << "Registering hotkey with Windows API...";
    BOOL result = RegisterHotKey(
        hwnd,
        m_hotkey.id,
        modifiersCode,
        vkCode
    );
    
    if (result) {
        qDebug() << "Hotkey registered successfully";
        m_hotkey.key = key;
        m_hotkey.modifiers = modifiers;
        m_hotkey.registered = true;
        qDebug() << "Hotkey data updated - registered:" << m_hotkey.registered;
        qInfo() << "Registered hotkey:" << QKeySequence(key | modifiers).toString();
        qDebug() << "HotkeyManager::registerHotkey() - EXIT (returning true)";
        return true;
    } else {
        DWORD error = GetLastError();
        qWarning() << "Failed to register hotkey - Windows error code:" << error;
        qCritical() << "Hotkey registration failed - hotkey may not work";
        qDebug() << "HotkeyManager::registerHotkey() - EXIT (returning false)";
        return false;
    }
#else
    // Linux implementation using QxtGlobalShortcut
    qDebug() << "Non-Windows platform detected - hotkey registration not implemented";
    Q_UNUSED(key);
    Q_UNUSED(modifiers);
    qWarning() << "Hotkey registration not supported on this platform";
    qDebug() << "HotkeyManager::registerHotkey() - EXIT (returning false)";
    return false;
#endif
}

bool HotkeyManager::unregisterHotkey() {
    qDebug() << "HotkeyManager::unregisterHotkey() - ENTRY";
    
    if (!m_hotkey.registered) {
        qDebug() << "No hotkey registered - nothing to do";
        qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning true)";
        return true;
    }
    
    qDebug() << "Unregistering hotkey with ID:" << m_hotkey.id;
    
#ifdef _WIN32
    HWND hwnd = nullptr;
    qDebug() << "Getting main window handle for hotkey unregistration...";
    if (qApp && qApp->activeWindow()) {
        hwnd = (HWND)qApp->activeWindow()->winId();
        qDebug() << "Window handle obtained:" << (void*)hwnd;
    } else {
        qWarning() << "No active window found - using null window handle";
    }
    
    qDebug() << "Unregistering hotkey with Windows API...";
    BOOL result = UnregisterHotKey(hwnd, m_hotkey.id);
    
    if (result) {
        qDebug() << "Hotkey unregistered successfully";
        m_hotkey.registered = false;
        qDebug() << "Hotkey data updated - registered:" << m_hotkey.registered;
        qInfo() << "Unregistered hotkey";
        qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning true)";
        return true;
    } else {
        DWORD error = GetLastError();
        qWarning() << "Failed to unregister hotkey - Windows error code:" << error;
        qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning false)";
        return false;
    }
#else
    qDebug() << "Non-Windows platform detected - hotkey unregistration not implemented";
    qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning false)";
    return false;
#endif
}

void HotkeyManager::setEnabled(bool enabled) {
    qDebug() << "HotkeyManager::setEnabled() - ENTRY";
    qDebug() << "New enabled state:" << (enabled ? "enabled" : "disabled");
    
    m_enabled = enabled;
    qDebug() << "Hotkey enabled state updated";
    
    qDebug() << "HotkeyManager::setEnabled() - EXIT";
}

bool HotkeyManager::nativeEventFilter(const QByteArray& eventType,
                                       void* message,
                                       qintptr* result) {
#ifdef _WIN32
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_HOTKEY && m_enabled && m_hotkey.registered) {
            qDebug() << "Hotkey event received";
            qDebug() << "Hotkey ID:" << (int)msg->wParam;
            qDebug() << "Expected ID:" << m_hotkey.id;
            
            if (msg->wParam == m_hotkey.id) {
                qDebug() << "Hotkey ID matches - emitting hotkeyPressed signal";
                emit hotkeyPressed();
                qDebug() << "hotkeyPressed signal emitted";
                return true;
            } else {
                qDebug() << "Hotkey ID does not match - ignoring event";
            }
        }
    }
#endif
    
    return false;
}
