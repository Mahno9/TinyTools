#include "HotkeyManager.h"
#include <QApplication>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#endif

int HotkeyManager::s_hotkeyIdCounter = 0;

HotkeyManager::HotkeyManager(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
{
    m_hotkeys[HotkeyType::MainToggle] = {0, 0, Qt::NoModifier, false};
    m_hotkeys[HotkeyType::AlternativeToggle] = {0, 0, Qt::NoModifier, false};

    qApp->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager() {
    unregisterAll();
    qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::unregisterHotkeyInternal(const HotkeyManager::HotkeyData& hotkey) {
#ifdef _WIN32
    // NULL HWND: thread-associated hotkey, matches how it was registered.
    if (UnregisterHotKey(nullptr, hotkey.id)) {
        return true;
    }
    DWORD error = GetLastError();
    // 1419 = hotkey is not registered - fine when cleaning up.
    if (error == 1419) {
        return true;
    }
    qWarning() << "Failed to unregister hotkey, Windows error:" << error;
    return false;
#else
    Q_UNUSED(hotkey);
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
    HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        qWarning() << "Invalid hotkey type:" << type;
        return false;
    }

    if (hotkey->registered) {
        unregisterHotkeyInternal(*hotkey);
        hotkey->registered = false;
    }

#ifdef _WIN32
    UINT vkCode = static_cast<UINT>(key);
    UINT modifiersCode = 0;
    if (modifiers & Qt::ControlModifier) modifiersCode |= MOD_CONTROL;
    if (modifiers & Qt::AltModifier)     modifiersCode |= MOD_ALT;
    if (modifiers & Qt::ShiftModifier)   modifiersCode |= MOD_SHIFT;
    if (modifiers & Qt::MetaModifier)    modifiersCode |= MOD_WIN;

    // NULL HWND: thread-associated hotkey - WM_HOTKEY is posted to the UI
    // thread's message queue regardless of which window is active, which is
    // more reliable than binding to a specific (potentially transient) HWND.
    hotkey->id = ++s_hotkeyIdCounter;

    if (RegisterHotKey(nullptr, hotkey->id, modifiersCode, vkCode)) {
        hotkey->key = key;
        hotkey->modifiers = modifiers;
        hotkey->registered = true;
        qInfo() << "Registered hotkey:" << HotkeyType::toDisplayName(type)
                << "as:" << QKeySequence(key | modifiers).toString();
        return true;
    }

    DWORD error = GetLastError();
    qCritical() << "RegisterHotKey failed for"
                << QKeySequence(key | modifiers).toString() << "- error:" << error
                << (error == 1409 ? "(already registered by another application)" : "");
    return false;
#else
    Q_UNUSED(key);
    Q_UNUSED(modifiers);
    qWarning() << "Hotkey registration not supported on this platform";
    return false;
#endif
}

bool HotkeyManager::unregisterHotkey(HotkeyType::Type type) {
    HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        qWarning() << "Hotkey type not found:" << type;
        return false;
    }

    if (!hotkey->registered) {
        return true;
    }

    bool result = unregisterHotkeyInternal(*hotkey);
    hotkey->registered = false;
    return result;
}

void HotkeyManager::unregisterAll() {
    for (auto it = m_hotkeys.begin(); it != m_hotkeys.end(); ++it) {
        if (it.value().registered) {
            unregisterHotkeyInternal(it.value());
            it.value().registered = false;
        }
    }
}

bool HotkeyManager::updateHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers) {
    HotkeyData* hotkey = getHotkeyData(type);
    if (!hotkey) {
        qWarning() << "Hotkey type not found:" << type;
        return false;
    }

    // Unchanged and still registered: nothing to do. This keeps unrelated
    // config saves (e.g. window geometry) from churning global hotkeys.
    if (hotkey->registered && hotkey->key == key && hotkey->modifiers == modifiers) {
        return true;
    }

    unregisterHotkey(type);
    return registerHotkey(type, key, modifiers);
}

bool HotkeyManager::isHotkeyRegistered(HotkeyType::Type type) const {
    const HotkeyData* hotkey = getHotkeyData(type);
    return hotkey && hotkey->registered;
}

int HotkeyManager::getHotkeyKey(HotkeyType::Type type) const {
    const HotkeyData* hotkey = getHotkeyData(type);
    return hotkey ? hotkey->key : 0;
}

Qt::KeyboardModifiers HotkeyManager::getHotkeyModifiers(HotkeyType::Type type) const {
    const HotkeyData* hotkey = getHotkeyData(type);
    return hotkey ? hotkey->modifiers : Qt::NoModifier;
}

void HotkeyManager::setEnabled(bool enabled) {
    m_enabled = enabled;
}

bool HotkeyManager::nativeEventFilter(const QByteArray& eventType,
                                       void* message,
                                       qintptr* result) {
    Q_UNUSED(result);
#ifdef _WIN32
    if (eventType == "windows_generic_MSG" && m_enabled) {
        MSG* msg = static_cast<MSG*>(message);

        if (msg->message == WM_HOTKEY) {
            for (auto it = m_hotkeys.constBegin(); it != m_hotkeys.constEnd(); ++it) {
                const HotkeyData& hotkey = it.value();
                if (hotkey.registered && msg->wParam == static_cast<WPARAM>(hotkey.id)) {
                    emit hotkeyPressed(it.key());
                    return true;
                }
            }
        }
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
#endif

    return false;
}
