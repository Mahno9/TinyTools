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
    
    // Initialize hotkeys map with default values
    m_hotkeys[HotkeyType::MainToggle] = {0, 0, Qt::NoModifier, false, nullptr};
    m_hotkeys[HotkeyType::ShowAndTranslate] = {0, 0, Qt::NoModifier, false, nullptr};
    qDebug() << "Hotkey data initialized for all hotkey types";
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
    
    qDebug() << "Unregistering all registered hotkeys...";
    unregisterAll();
    qDebug() << "All hotkeys unregistered";
    
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

bool HotkeyManager::unregisterHotkeyInternal(const HotkeyManager::HotkeyData& hotkey) {
    qDebug() << "HotkeyManager::unregisterHotkeyInternal() - ENTRY";
    qDebug() << "Using window handle:" << hotkey.windowHandle;
    qDebug() << "Unregistering hotkey with ID:" << hotkey.id;
    
#ifdef _WIN32
    qDebug() << "Unregistering hotkey with Windows API...";
    HWND hwnd = static_cast<HWND>(hotkey.windowHandle);
    BOOL result = UnregisterHotKey(hwnd, hotkey.id);
    
    if (result) {
        qDebug() << "Hotkey unregistered successfully";
        qInfo() << "Unregistered hotkey";
        qDebug() << "HotkeyManager::unregisterHotkeyInternal() - EXIT (returning true)";
        return true;
    } else {
        DWORD error = GetLastError();
        qWarning() << "Failed to unregister hotkey - Windows error code:" << error;
        // Error 1419 means "hotkey is not registered" - this is okay if we're just cleaning up
        if (error == 1419) {
            qDebug() << "Hotkey was not actually registered (error 1419) - this is acceptable";
            qDebug() << "HotkeyManager::unregisterHotkeyInternal() - EXIT (returning true)";
            return true;  // Don't treat this as a failure
        }
        qDebug() << "HotkeyManager::unregisterHotkeyInternal() - EXIT (returning false)";
        return false;
    }
#else
    qDebug() << "Non-Windows platform detected - hotkey unregistration not implemented";
    qDebug() << "HotkeyManager::unregisterHotkeyInternal() - EXIT (returning false)";
    return false;
#endif
}

HotkeyManager::HotkeyData* HotkeyManager::getHotkeyData(HotkeyType::Type type) {
    auto it = m_hotkeys.find(type);
    if (it == m_hotkeys.end()) {
        return nullptr;
    }
    return &it.value();
}

const HotkeyManager::HotkeyData* HotkeyManager::getHotkeyData(HotkeyType::Type type) const {
    auto it = m_hotkeys.find(type);
    if (it == m_hotkeys.end()) {
        return nullptr;
    }
    return &it.value();
}

bool HotkeyManager::registerHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::registerHotkey() - ENTRY";
    qDebug() << "Type:" << HotkeyType::toDisplayName(type);
    qDebug() << "Key:" << key;
    qDebug() << "Modifiers raw value:" << static_cast<int>(modifiers);
    qDebug() << "Modifiers as string:" << QKeySequence(key | modifiers).toString();
    
    // Validate type
    if (type < 0 || type >= HotkeyType::Count) {
        qWarning() << "Invalid hotkey type:" << type;
        return false;
    }
    
    // Detailed modifier breakdown
    qDebug() << "  Qt::ControlModifier present:" << ((modifiers & Qt::ControlModifier) ? "YES" : "NO");
    qDebug() << "  Qt::AltModifier present:" << ((modifiers & Qt::AltModifier) ? "YES" : "NO");
    qDebug() << "  Qt::ShiftModifier present:" << ((modifiers & Qt::ShiftModifier) ? "YES" : "NO");
    
    // Get or create hotkey data
    auto it = m_hotkeys.find(type);
    HotkeyManager::HotkeyData& hotkey = it.value();
    
    // Unregister existing if already registered
    if (hotkey.registered) {
        qDebug() << "Unregistering existing hotkey before registering new one...";
        qDebug() << "Using stored window handle for unregistration:" << hotkey.windowHandle;
        unregisterHotkeyInternal(hotkey);
        hotkey.registered = false;
        qDebug() << "Existing hotkey unregistered";
    }
    
#ifdef _WIN32
    qDebug() << "Windows platform detected - using Windows hotkey API";
    
    UINT vkCode = static_cast<UINT>(key);
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
    if (modifiers & Qt::MetaModifier) {
        modifiersCode |= MOD_WIN;
        qDebug() << "  Qt::MetaModifier detected - OR-ing with MOD_WIN (0x" << QString::number(MOD_WIN, 16) << ")";
        qDebug() << "  Current modifiersCode: 0x" << QString::number(modifiersCode, 16);
    }
    qDebug() << "=== FINAL MODIFIER FLAGS ===";
    qDebug() << "Final modifiersCode: 0x" << QString::number(modifiersCode, 16) << "(decimal:" << modifiersCode << ")";
    qDebug() << "Expected flags for Alt+Ctrl: 0x" << QString::number(MOD_ALT | MOD_CONTROL, 16);
    
    // Get and store window handle
    hotkey.windowHandle = getMainWindowHandle();
    qDebug() << "Window handle for registration:" << hotkey.windowHandle;
    
    // Register hotkey with Windows
    hotkey.id = ++s_hotkeyIdCounter;
    qDebug() << "New hotkey ID:" << hotkey.id;
    
    qDebug() << "=== REGISTERHOTKEY CALL ===";
    qDebug() << "Calling RegisterHotKey with parameters:";
    qDebug() << "  HWND (window handle):" << hotkey.windowHandle;
    qDebug() << "  ID:" << hotkey.id;
    qDebug() << "  fsModifiers (flags): 0x" << QString::number(modifiersCode, 16) << "(decimal:" << modifiersCode << ")";
    qDebug() << "  vk (virtual key code):" << vkCode << "(0x" << QString::number(vkCode, 16) << ")";
    
    HWND hwnd = static_cast<HWND>(hotkey.windowHandle);
    
    // Log window handle details before calling RegisterHotKey
    qDebug() << "=== WINDOW HANDLE DETAILS ===";
    qDebug() << "Window handle pointer:" << hotkey.windowHandle;
    qDebug() << "Window handle (HWND cast):" << hwnd;
    qDebug() << "Window handle is NULL:" << (hwnd == NULL ? "YES" : "NO");
    qDebug() << "Window handle in hex:" << QString::number((quintptr)hwnd, 16);
    qDebug() << "Is Win key modifier present:" << ((modifiersCode & MOD_WIN) ? "YES" : "NO");
    
    BOOL result = RegisterHotKey(
        hwnd,
        hotkey.id,
        modifiersCode,
        vkCode
    );
    
    qDebug() << "RegisterHotKey returned:" << (result ? "TRUE (success)" : "FALSE (failure)");
    
    if (result) {
        qDebug() << "Hotkey registered successfully";
        hotkey.key = key;
        hotkey.modifiers = modifiers;
        hotkey.registered = true;
        qDebug() << "Hotkey data updated - registered:" << hotkey.registered;
        qDebug() << "Stored window handle:" << hotkey.windowHandle;
        qInfo() << "Registered hotkey:" << HotkeyType::toDisplayName(type)
                << "as:" << QKeySequence(key | modifiers).toString();
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
            result = RegisterHotKey(nullHwnd, hotkey.id, modifiersCode, vkCode);
            
            qWarning() << "RegisterHotKey with NULL handle returned:" << (result ? "TRUE (success)" : "FALSE (failure)");
            
            if (result) {
                qWarning() << "=== WIN KEY WORKAROUND SUCCESSFUL ===";
                qWarning() << "Hotkey registered successfully with NULL window handle";
                qWarning() << "This is a workaround for Windows API Win key restrictions";
                hotkey.key = key;
                hotkey.modifiers = modifiers;
                hotkey.registered = true;
                hotkey.windowHandle = nullptr; // Store NULL handle
                qInfo() << "Registered hotkey (with Win key workaround):" << HotkeyType::toDisplayName(type)
                        << "as:" << QKeySequence(key | modifiers).toString();
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
    Q_UNUSED(key);
    Q_UNUSED(modifiers);
    qDebug() << "Non-Windows platform detected - hotkey registration not implemented";
    qWarning() << "Hotkey registration not supported on this platform";
    qDebug() << "HotkeyManager::registerHotkey() - EXIT (returning false)";
    return false;
#endif
}

bool HotkeyManager::unregisterHotkey(HotkeyType::Type type) {
    qDebug() << "HotkeyManager::unregisterHotkey() - ENTRY";
    qDebug() << "Type:" << HotkeyType::toDisplayName(type);
    
    HotkeyManager::HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        qWarning() << "Hotkey type not found:" << type;
        qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning false)";
        return false;
    }
    
    if (!hotkey->registered) {
        qDebug() << "Hotkey not registered - nothing to do";
        qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning true)";
        return true;
    }
    
    qDebug() << "Unregistering hotkey...";
    qDebug() << "Using stored window handle for unregistration:" << hotkey->windowHandle;
    bool result = unregisterHotkeyInternal(*hotkey);
    hotkey->registered = false;
    hotkey->windowHandle = nullptr;
    qDebug() << "Hotkey unregistered";
    
    qDebug() << "HotkeyManager::unregisterHotkey() - EXIT (returning" << (result ? "true" : "false") << ")";
    return result;
}

void HotkeyManager::unregisterAll() {
    qDebug() << "HotkeyManager::unregisterAll() - ENTRY";
    
    for (auto it = m_hotkeys.begin(); it != m_hotkeys.end(); ++it) {
        HotkeyType::Type type = it.key();
        HotkeyManager::HotkeyData& hotkey = it.value();
        
        if (hotkey.registered) {
            qDebug() << "Unregistering hotkey:" << HotkeyType::toDisplayName(type);
            unregisterHotkeyInternal(hotkey);
            hotkey.registered = false;
            hotkey.windowHandle = nullptr;
        }
    }
    
    qDebug() << "All hotkeys unregistered";
    qDebug() << "HotkeyManager::unregisterAll() - EXIT";
}

void HotkeyManager::updateHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers) {
    qDebug() << "HotkeyManager::updateHotkey() - ENTRY";
    qDebug() << "Type:" << HotkeyType::toDisplayName(type);
    qDebug() << "New key:" << key;
    qDebug() << "New modifiers:" << QKeySequence(key | modifiers).toString();
    
    if (!m_hotkeys.contains(type)) {
        qWarning() << "Hotkey type not found:" << type;
        return;
    }
    
    unregisterHotkey(type);
    registerHotkey(type, key, modifiers);
    
    qDebug() << "HotkeyManager::updateHotkey() - EXIT";
}

bool HotkeyManager::isHotkeyRegistered(HotkeyType::Type type) const {
    const HotkeyManager::HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        return false;
    }
    return hotkey->registered;
}

int HotkeyManager::getHotkeyKey(HotkeyType::Type type) const {
    const HotkeyManager::HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        return 0;
    }
    return hotkey->key;
}

Qt::KeyboardModifiers HotkeyManager::getHotkeyModifiers(HotkeyType::Type type) const {
    const HotkeyManager::HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        return Qt::NoModifier;
    }
    return hotkey->modifiers;
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
    if (eventType == "windows_generic_MSG" && m_enabled) {
        MSG* msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_HOTKEY) {
            // Find hotkey by ID
            for (auto it = m_hotkeys.constBegin(); it != m_hotkeys.constEnd(); ++it) {
                const HotkeyManager::HotkeyData& hotkey = it.value();
                if (hotkey.registered && msg->wParam == hotkey.id) {
                    qDebug() << "Hotkey pressed:" << HotkeyType::toDisplayName(it.key());
                    qDebug() << "Emitting hotkeyPressed signal with type:" << it.key();
                    emit hotkeyPressed(it.key());
                    qDebug() << "hotkeyPressed signal emitted";
                    return true;
                }
            }
        }
    }
#endif
    
    return false;
}
