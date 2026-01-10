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
    m_hotkey.windowHandle = nullptr;  // Initialize window handle
    qDebug() << "Hotkey data initialized";
    
    // Initialize show and translate hotkey
    m_showTranslateHotkey.id = 0;
    m_showTranslateHotkey.key = 0;
    m_showTranslateHotkey.modifiers = Qt::NoModifier;
    m_showTranslateHotkey.registered = false;
    m_showTranslateHotkey.windowHandle = nullptr;
    qDebug() << "Show and Translate hotkey data initialized";
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

void* HotkeyManager::getMainWindowHandle() {
    qDebug() << "Getting main window handle...";
    
#ifdef _WIN32
    // Try to get MainWindow specifically, not just any active window
    // This ensures we always use the same window handle
    QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* widget : topLevelWidgets) {
        if (widget->objectName() == "MainWindow" || widget->inherits("MainWindow")) {
            void* hwnd = (void*)widget->winId();
            qDebug() << "MainWindow handle found:" << hwnd;
            return hwnd;
        }
    }
    
    // Fallback to active window if MainWindow not found
    if (qApp && qApp->activeWindow()) {
        void* hwnd = (void*)qApp->activeWindow()->winId();
        qDebug() << "Active window handle (fallback):" << hwnd;
        return hwnd;
    }
    
    qWarning() << "No window found - using null window handle";
    qDebug() << "Returning null window handle";
    return nullptr;
#else
    return nullptr;
#endif
}

bool HotkeyManager::unregisterHotkeyWithHandle(void* hwnd, int hotkeyId) {
    qDebug() << "HotkeyManager::unregisterHotkeyWithHandle() - ENTRY";
    qDebug() << "Using window handle:" << hwnd;
    qDebug() << "Unregistering hotkey with ID:" << hotkeyId;
    
#ifdef _WIN32
    qDebug() << "Unregistering hotkey with Windows API...";
    BOOL result = UnregisterHotKey((HWND)hwnd, hotkeyId);
    
    if (result) {
        qDebug() << "Hotkey unregistered successfully";
        if (hotkeyId == m_hotkey.id) {
            m_hotkey.registered = false;
            m_hotkey.windowHandle = nullptr;
            qDebug() << "Hotkey data updated - registered:" << m_hotkey.registered;
        } else if (hotkeyId == m_showTranslateHotkey.id) {
            m_showTranslateHotkey.registered = false;
            m_showTranslateHotkey.windowHandle = nullptr;
            qDebug() << "Show and Translate hotkey data updated - registered:" << m_showTranslateHotkey.registered;
        }
        qInfo() << "Unregistered hotkey";
        qDebug() << "HotkeyManager::unregisterHotkeyWithHandle() - EXIT (returning true)";
        return true;
    } else {
        DWORD error = GetLastError();
        qWarning() << "Failed to unregister hotkey - Windows error code:" << error;
        // Error 1419 means "hotkey is not registered" - this is okay if we're just cleaning up
        if (error == 1419) {
            qDebug() << "Hotkey was not actually registered (error 1419) - this is acceptable";
            if (hotkeyId == m_hotkey.id) {
                m_hotkey.registered = false;
                m_hotkey.windowHandle = nullptr;
            } else if (hotkeyId == m_showTranslateHotkey.id) {
                m_showTranslateHotkey.registered = false;
                m_showTranslateHotkey.windowHandle = nullptr;
            }
            qDebug() << "HotkeyManager::unregisterHotkeyWithHandle() - EXIT (returning true)";
            return true;  // Don't treat this as a failure
        }
        qDebug() << "HotkeyManager::unregisterHotkeyWithHandle() - EXIT (returning false)";
        return false;
    }
#else
    qDebug() << "Non-Windows platform detected - hotkey unregistration not implemented";
    qDebug() << "HotkeyManager::unregisterHotkeyWithHandle() - EXIT (returning false)";
    return false;
#endif
}

bool HotkeyManager::registerHotkey(int key, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::registerHotkey() - ENTRY";
    qDebug() << "Key:" << key;
    qDebug() << "Modifiers raw value:" << static_cast<int>(modifiers);
    qDebug() << "Modifiers as string:" << QKeySequence(key | modifiers).toString();
    
    // Detailed modifier breakdown
    qDebug() << "  Qt::ControlModifier present:" << ((modifiers & Qt::ControlModifier) ? "YES" : "NO");
    qDebug() << "  Qt::AltModifier present:" << ((modifiers & Qt::AltModifier) ? "YES" : "NO");
    qDebug() << "  Qt::ShiftModifier present:" << ((modifiers & Qt::ShiftModifier) ? "YES" : "NO");
    
    // Unregister existing hotkey first using stored window handle
    if (m_hotkey.registered) {
        qDebug() << "Unregistering existing hotkey before registering new one...";
        qDebug() << "Using stored window handle for unregistration:" << m_hotkey.windowHandle;
        unregisterHotkeyWithHandle(m_hotkey.windowHandle, m_hotkey.id);
        qDebug() << "Existing hotkey unregistered";
    }
    
#ifdef _WIN32
    qDebug() << "Windows platform detected - using Windows hotkey API";
    
    UINT vkCode = key;
    UINT modifiersCode = 0;
    
    qDebug() << "=== MODIFIER FLAG CALCULATION ===";
    qDebug() << "Starting with modifiersCode = 0";
    if (modifiers & Qt::ControlModifier) {
        modifiersCode |= MOD_CONTROL;
        qDebug() << "  Qt::ControlModifier detected - OR-ing with MOD_CONTROL (0x" << QString::number(MOD_CONTROL, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    if (modifiers & Qt::AltModifier) {
        modifiersCode |= MOD_ALT;
        qDebug() << "  Qt::AltModifier detected - OR-ing with MOD_ALT (0x" << QString::number(MOD_ALT, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    if (modifiers & Qt::ShiftModifier) {
        modifiersCode |= MOD_SHIFT;
        qDebug() << "  Qt::ShiftModifier detected - OR-ing with MOD_SHIFT (0x" << QString::number(MOD_SHIFT, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    qDebug() << "=== FINAL MODIFIER FLAGS ===";
    qDebug() << "Final modifiersCode: 0x" << QString::number(modifiersCode, 16) << "(decimal:" << modifiersCode << ")";
    qDebug() << "Expected flags for Alt+Ctrl: 0x" << QString::number(MOD_ALT | MOD_CONTROL, 16);
    
    // Get and store window handle
    m_hotkey.windowHandle = getMainWindowHandle();
    qDebug() << "Window handle for registration:" << m_hotkey.windowHandle;
    
    // Register hotkey with Windows
    m_hotkey.id = ++s_hotkeyIdCounter;
    qDebug() << "New hotkey ID:" << m_hotkey.id;
    
    qDebug() << "=== REGISTERHOTKEY CALL ===";
    qDebug() << "Calling RegisterHotKey with parameters:";
    qDebug() << "  HWND (window handle):" << m_hotkey.windowHandle;
    qDebug() << "  ID:" << m_hotkey.id;
    qDebug() << "  fsModifiers (flags): 0x" << QString::number(modifiersCode, 16) << "(decimal:" << modifiersCode << ")";
    qDebug() << "  vk (virtual key code):" << vkCode << "(0x" << QString::number(vkCode, 16) << ")";
    qDebug() << "Expected for Alt+Win+T:";
    qDebug() << "  fsModifiers should be: 0x" << QString::number(MOD_ALT | MOD_WIN, 16);
    qDebug() << "  vk should be: 84 (0x54 for 'T')";
    
    HWND hwnd = (HWND)m_hotkey.windowHandle;
    
    // Log window handle details before calling RegisterHotKey
    qDebug() << "=== WINDOW HANDLE DETAILS ===";
    qDebug() << "Window handle pointer:" << m_hotkey.windowHandle;
    qDebug() << "Window handle (HWND cast):" << hwnd;
    qDebug() << "Window handle is NULL:" << (hwnd == NULL ? "YES" : "NO");
    qDebug() << "Window handle in hex:" << QString::number((quintptr)hwnd, 16);
    qDebug() << "Is Win key modifier present:" << ((modifiersCode & MOD_WIN) ? "YES" : "NO");
    
    BOOL result = RegisterHotKey(
        hwnd,
        m_hotkey.id,
        modifiersCode,
        vkCode
    );
    
    qDebug() << "RegisterHotKey returned:" << (result ? "TRUE (success)" : "FALSE (failure)");
    
    if (result) {
        qDebug() << "Hotkey registered successfully";
        m_hotkey.key = key;
        m_hotkey.modifiers = modifiers;
        m_hotkey.registered = true;
        qDebug() << "Hotkey data updated - registered:" << m_hotkey.registered;
        qDebug() << "Stored window handle:" << m_hotkey.windowHandle;
        qInfo() << "Registered hotkey:" << QKeySequence(key | modifiers).toString();
        qDebug() << "HotkeyManager::registerHotkey() - EXIT (returning true)";
        return true;
    } else {
        DWORD error = GetLastError();
        qCritical() << "=== REGISTERHOTKEY FAILED ===";
        qCritical() << "Windows error code:" << error;
        qCritical() << "Error code in hex: 0x" << QString::number(error, 16);
        
        // Detailed error descriptions
        switch (error) {
            case 1409: // ERROR_HOTKEY_ALREADY_REGISTERED
                qCritical() << "ERROR_HOTKEY_ALREADY_REGISTERED (1409) - Hotkey already registered by this or another application";
                qCritical() << "You may need to restart the application to clear old registration";
                break;
            case 87: // ERROR_INVALID_PARAMETER
                qCritical() << "ERROR_INVALID_PARAMETER (87) - Invalid parameter passed to RegisterHotKey";
                qCritical() << "Check if window handle is valid:" << (hwnd != nullptr ? "YES" : "NULL");
                qCritical() << "Check if modifiers are valid (0x" << QString::number(modifiersCode, 16) << ")";
                qCritical() << "Check if virtual key code is valid:" << vkCode;
                break;
            default:
                qCritical() << "Unknown error - Error code:" << error;
                break;
        }
        
        // WORKAROUND: Try registering with NULL window handle if Win key is present
        // Windows API sometimes fails with real window handles for Win key combinations
        if ((modifiersCode & MOD_WIN) && hwnd != nullptr) {
            qWarning() << "=== ATTEMPTING WIN KEY WORKAROUND ===";
            qWarning() << "Win key modifier detected and registration failed with real window handle";
            qWarning() << "Attempting to register with NULL window handle instead...";
            
            HWND nullHwnd = NULL;
            result = RegisterHotKey(nullHwnd, m_hotkey.id, modifiersCode, vkCode);
            
            qWarning() << "RegisterHotKey with NULL handle returned:" << (result ? "TRUE (success)" : "FALSE (failure)");
            
            if (result) {
                qWarning() << "=== WIN KEY WORKAROUND SUCCESSFUL ===";
                qWarning() << "Hotkey registered successfully with NULL window handle";
                qWarning() << "This is a workaround for Windows API Win key restrictions";
                m_hotkey.key = key;
                m_hotkey.modifiers = modifiers;
                m_hotkey.registered = true;
                m_hotkey.windowHandle = nullptr; // Store NULL handle
                qInfo() << "Registered hotkey (with Win key workaround):" << QKeySequence(key | modifiers).toString();
                qDebug() << "HotkeyManager::registerHotkey() - EXIT (returning true)";
                return true;
            } else {
                DWORD nullError = GetLastError();
                qCritical() << "=== WIN KEY WORKAROUND FAILED ===";
                qCritical() << "Registration with NULL handle also failed";
                qCritical() << "Windows error code:" << nullError;
                qCritical() << "Error code in hex: 0x" << QString::number(nullError, 16);
            }
        }
        
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

bool HotkeyManager::registerShowTranslateHotkey(int key, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::registerShowTranslateHotkey() - ENTRY";
    qDebug() << "Key:" << key;
    qDebug() << "Modifiers raw value:" << static_cast<int>(modifiers);
    qDebug() << "Modifiers as string:" << QKeySequence(key | modifiers).toString();
    
    // Detailed modifier breakdown
    qDebug() << "  Qt::ControlModifier present:" << ((modifiers & Qt::ControlModifier) ? "YES" : "NO");
    qDebug() << "  Qt::AltModifier present:" << ((modifiers & Qt::AltModifier) ? "YES" : "NO");
    qDebug() << "  Qt::ShiftModifier present:" << ((modifiers & Qt::ShiftModifier) ? "YES" : "NO");
    
    // Unregister existing show and translate hotkey first using stored window handle
    if (m_showTranslateHotkey.registered) {
        qDebug() << "Unregistering existing show and translate hotkey before registering new one...";
        qDebug() << "Using stored window handle for unregistration:" << m_showTranslateHotkey.windowHandle;
        unregisterHotkeyWithHandle(m_showTranslateHotkey.windowHandle, m_showTranslateHotkey.id);
        qDebug() << "Existing show and translate hotkey unregistered";
    }
    
#ifdef _WIN32
    qDebug() << "Windows platform detected - using Windows hotkey API";
    
    UINT vkCode = key;
    UINT modifiersCode = 0;
    
    qDebug() << "=== MODIFIER FLAG CALCULATION ===";
    qDebug() << "Starting with modifiersCode = 0";
    if (modifiers & Qt::ControlModifier) {
        modifiersCode |= MOD_CONTROL;
        qDebug() << "  Qt::ControlModifier detected - OR-ing with MOD_CONTROL (0x" << QString::number(MOD_CONTROL, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    if (modifiers & Qt::AltModifier) {
        modifiersCode |= MOD_ALT;
        qDebug() << "  Qt::AltModifier detected - OR-ing with MOD_ALT (0x" << QString::number(MOD_ALT, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    if (modifiers & Qt::ShiftModifier) {
        modifiersCode |= MOD_SHIFT;
        qDebug() << "  Qt::ShiftModifier detected - OR-ing with MOD_SHIFT (0x" << QString::number(MOD_SHIFT, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    qDebug() << "=== FINAL MODIFIER FLAGS ===";
    qDebug() << "Final modifiersCode: 0x" << QString::number(modifiersCode, 16) << "(decimal:" << modifiersCode << ")";
    
    // Get and store window handle
    m_showTranslateHotkey.windowHandle = getMainWindowHandle();
    qDebug() << "Window handle for registration:" << m_showTranslateHotkey.windowHandle;
    
    // Register hotkey with Windows
    m_showTranslateHotkey.id = ++s_hotkeyIdCounter;
    qDebug() << "New show and translate hotkey ID:" << m_showTranslateHotkey.id;
    
    qDebug() << "=== REGISTERHOTKEY CALL (Show and Translate) ===";
    qDebug() << "Calling RegisterHotKey with parameters:";
    qDebug() << "  HWND (window handle):" << m_showTranslateHotkey.windowHandle;
    qDebug() << "  ID:" << m_showTranslateHotkey.id;
    qDebug() << "  fsModifiers (flags): 0x" << QString::number(modifiersCode, 16) << "(decimal:" << modifiersCode << ")";
    qDebug() << "  vk (virtual key code):" << vkCode << "(0x" << QString::number(vkCode, 16) << ")";
    
    HWND hwnd = (HWND)m_showTranslateHotkey.windowHandle;
    
    BOOL result = RegisterHotKey(
        hwnd,
        m_showTranslateHotkey.id,
        modifiersCode,
        vkCode
    );
    
    qDebug() << "RegisterHotKey returned:" << (result ? "TRUE (success)" : "FALSE (failure)");
    
    if (result) {
        qDebug() << "Show and Translate hotkey registered successfully";
        m_showTranslateHotkey.key = key;
        m_showTranslateHotkey.modifiers = modifiers;
        m_showTranslateHotkey.registered = true;
        qDebug() << "Show and Translate hotkey data updated - registered:" << m_showTranslateHotkey.registered;
        qDebug() << "Stored window handle:" << m_showTranslateHotkey.windowHandle;
        qInfo() << "Registered show and translate hotkey:" << QKeySequence(key | modifiers).toString();
        qDebug() << "HotkeyManager::registerShowTranslateHotkey() - EXIT (returning true)";
        return true;
    } else {
        DWORD error = GetLastError();
        qCritical() << "=== REGISTERHOTKEY FAILED (Show and Translate) ===";
        qCritical() << "Windows error code:" << error;
        qCritical() << "Error code in hex: 0x" << QString::number(error, 16);
        
        // Detailed error descriptions
        switch (error) {
            case 1409: // ERROR_HOTKEY_ALREADY_REGISTERED
                qCritical() << "ERROR_HOTKEY_ALREADY_REGISTERED (1409) - Hotkey already registered by this or another application";
                break;
            case 87: // ERROR_INVALID_PARAMETER
                qCritical() << "ERROR_INVALID_PARAMETER (87) - Invalid parameter passed to RegisterHotKey";
                break;
            default:
                qCritical() << "Unknown error - Error code:" << error;
                break;
        }
        
        qDebug() << "HotkeyManager::registerShowTranslateHotkey() - EXIT (returning false)";
        return false;
    }
#else
    qDebug() << "Non-Windows platform detected - show and translate hotkey registration not implemented";
    Q_UNUSED(key);
    Q_UNUSED(modifiers);
    qWarning() << "Show and translate hotkey registration not supported on this platform";
    qDebug() << "HotkeyManager::registerShowTranslateHotkey() - EXIT (returning false)";
    return false;
#endif
}

bool HotkeyManager::unregisterHotkey() {
    qDebug() << "HotkeyManager::unregisterHotkey() - ENTRY";
    
    // Unregister main hotkey
    if (m_hotkey.registered) {
        qDebug() << "Unregistering main hotkey...";
        qDebug() << "Using stored window handle for unregistration:" << m_hotkey.windowHandle;
        unregisterHotkeyWithHandle(m_hotkey.windowHandle, m_hotkey.id);
        qDebug() << "Main hotkey unregistered";
    }
    
    // Unregister show and translate hotkey
    if (m_showTranslateHotkey.registered) {
        qDebug() << "Unregistering show and translate hotkey...";
        qDebug() << "Using stored window handle for unregistration:" << m_showTranslateHotkey.windowHandle;
        unregisterHotkeyWithHandle(m_showTranslateHotkey.windowHandle, m_showTranslateHotkey.id);
        qDebug() << "Show and translate hotkey unregistered";
    }
    
    if (!m_hotkey.registered && !m_showTranslateHotkey.registered) {
        qDebug() << "No hotkeys registered - nothing to do";
    }
    
    qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning true)";
    return true;
}

void HotkeyManager::updateHotkey(int keyCode, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::updateHotkey() - ENTRY";
    qDebug() << "New key:" << keyCode;
    qDebug() << "New modifiers:" << QKeySequence(keyCode | modifiers).toString();
    
    // Unregister existing hotkey first using stored window handle
    if (m_hotkey.registered) {
        qDebug() << "Unregistering existing hotkey...";
        qDebug() << "Using stored window handle:" << m_hotkey.windowHandle;
        unregisterHotkeyWithHandle(m_hotkey.windowHandle, m_hotkey.id);
        qDebug() << "Existing hotkey unregistered";
    }
    
    // Register new hotkey
    qDebug() << "Registering new hotkey...";
    bool registered = registerHotkey(keyCode, modifiers);
    if (registered) {
        qDebug() << "Hotkey updated successfully";
        qInfo() << "Updated hotkey to:" << QKeySequence(keyCode | modifiers).toString();
    } else {
        qWarning() << "Failed to update hotkey";
    }
    
    qDebug() << "HotkeyManager::updateHotkey() - EXIT";
}

void HotkeyManager::updateShowTranslateHotkey(int keyCode, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::updateShowTranslateHotkey() - ENTRY";
    qDebug() << "New key:" << keyCode;
    qDebug() << "New modifiers:" << QKeySequence(keyCode | modifiers).toString();
    
    // Unregister existing show and translate hotkey first using stored window handle
    if (m_showTranslateHotkey.registered) {
        qDebug() << "Unregistering existing show and translate hotkey...";
        qDebug() << "Using stored window handle:" << m_showTranslateHotkey.windowHandle;
        unregisterHotkeyWithHandle(m_showTranslateHotkey.windowHandle, m_showTranslateHotkey.id);
        qDebug() << "Existing show and translate hotkey unregistered";
    }
    
    // Register new show and translate hotkey
    qDebug() << "Registering new show and translate hotkey...";
    bool registered = registerShowTranslateHotkey(keyCode, modifiers);
    if (registered) {
        qDebug() << "Show and Translate hotkey updated successfully";
        qInfo() << "Updated show and translate hotkey to:" << QKeySequence(keyCode | modifiers).toString();
    } else {
        qWarning() << "Failed to update show and translate hotkey";
    }
    
    qDebug() << "HotkeyManager::updateShowTranslateHotkey() - EXIT";
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
        
        if (msg->message == WM_HOTKEY && m_enabled) {
            qDebug() << "Hotkey event received";
            qDebug() << "Hotkey ID:" << (int)msg->wParam;
            qDebug() << "Expected main hotkey ID:" << m_hotkey.id;
            qDebug() << "Expected show and translate hotkey ID:" << m_showTranslateHotkey.id;
            
            if (m_hotkey.registered && msg->wParam == m_hotkey.id) {
                qDebug() << "Main hotkey ID matches - emitting hotkeyPressed signal";
                emit hotkeyPressed();
                qDebug() << "hotkeyPressed signal emitted";
                return true;
            } else if (m_showTranslateHotkey.registered && msg->wParam == m_showTranslateHotkey.id) {
                qDebug() << "Show and Translate hotkey ID matches - emitting showTranslateHotkeyPressed signal";
                emit showTranslateHotkeyPressed();
                qDebug() << "showTranslateHotkeyPressed signal emitted";
                return true;
            } else {
                qDebug() << "Hotkey ID does not match - ignoring event";
            }
        }
    }
#endif
    
    return false;
}
