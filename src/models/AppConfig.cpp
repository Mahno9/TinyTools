#include "AppConfig.h"
#include "../core/HotkeyManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QMutexLocker>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

// Static instance
QPointer<AppConfig> AppConfig::s_instance = nullptr;
QMutex AppConfig::s_mutex;

AppConfig* AppConfig::instance() {
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new AppConfig();
    }
    return s_instance;
}

AppConfig::AppConfig() {
    m_configPath = getConfigFilePath();
    resetToDefaults();
}

bool AppConfig::load() {
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file for reading:" << m_configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Config file parse error:" << error.errorString();
        return false;
    }
    
    m_config = doc.object();
    qInfo() << "Configuration loaded from:" << m_configPath;
    return true;
}

bool AppConfig::save() {
    qDebug() << "AppConfig::save() - Starting save operation";
    qDebug() << "Config file path:" << m_configPath;
    
    QJsonDocument doc(m_config);
    qDebug() << "JSON document created from config";
    
    // Ensure directory exists
    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists()) {
        qDebug() << "Config directory does not exist, creating:" << dir.path();
        if (!dir.mkpath(".")) {
            qCritical() << "Failed to create config directory:" << dir.path();
            qDebug() << "AppConfig::save() - Failed: Cannot create config directory";
            return false;
        }
        qDebug() << "Config directory created successfully";
    } else {
        qDebug() << "Config directory exists:" << dir.path();
    }
    
    QFile file(m_configPath);
    qDebug() << "Opening config file for writing:" << m_configPath;
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open config file for writing:" << m_configPath;
        qCritical() << "Error:" << file.errorString();
        qDebug() << "AppConfig::save() - Failed: Cannot open file for writing";
        return false;
    }
    qDebug() << "Config file opened successfully";
    
    QByteArray jsonData = doc.toJson();
    qDebug() << "JSON data size:" << jsonData.size() << "bytes";
    
    qint64 bytesWritten = file.write(jsonData);
    qDebug() << "Bytes written:" << bytesWritten;
    
    if (bytesWritten != jsonData.size()) {
        qCritical() << "Failed to write all data. Expected:" << jsonData.size() << "Written:" << bytesWritten;
        file.close();
        qDebug() << "AppConfig::save() - Failed: Incomplete write";
        return false;
    }
    
    qDebug() << "Flushing file to disk...";
    if (!file.flush()) {
        qCritical() << "Failed to flush file to disk:" << m_configPath;
        file.close();
        qDebug() << "AppConfig::save() - Failed: Flush operation failed";
        return false;
    }
    qDebug() << "File flushed successfully";
    
    file.close();
    qDebug() << "File closed";
    
    // Verify file was written successfully
    qDebug() << "Verifying file was written...";
    QFile verifyFile(m_configPath);
    if (!verifyFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open file for verification:" << m_configPath;
        qCritical() << "Error:" << verifyFile.errorString();
        qDebug() << "AppConfig::save() - Failed: Cannot verify file (cannot open for reading)";
        return false;
    }
    
    QByteArray verifyData = verifyFile.readAll();
    verifyFile.close();
    
    if (verifyData.isEmpty()) {
        qCritical() << "Verification failed: File is empty:" << m_configPath;
        qDebug() << "AppConfig::save() - Failed: File is empty after write";
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument verifyDoc = QJsonDocument::fromJson(verifyData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qCritical() << "Verification failed: File contains invalid JSON:" << parseError.errorString();
        qDebug() << "AppConfig::save() - Failed: Invalid JSON after write";
        return false;
    }
    
    qDebug() << "Verification successful: File contains valid JSON";
    qDebug() << "File size after verification:" << verifyData.size() << "bytes";
    
    qInfo() << "Configuration saved successfully to:" << m_configPath;
    emit settingsChanged();
    qDebug() << "AppConfig::save() - Completed successfully";
    return true;
}

void AppConfig::resetToDefaults() {
    // Initialize all hotkeys using loop
    for (int i = 0; i < HotkeyType::Count; ++i) {
        HotkeyType::Type type = static_cast<HotkeyType::Type>(i);
        
        int defaultKey;
        Qt::KeyboardModifiers defaultModifiers;
        
        switch (type) {
            case HotkeyType::MainToggle:
                defaultKey = Qt::Key_T;
                defaultModifiers = Qt::ControlModifier | Qt::AltModifier;
                break;
            case HotkeyType::AlternativeToggle:
                defaultKey = Qt::Key_S;
                defaultModifiers = Qt::ControlModifier | Qt::AltModifier;
                break;
            default:
                defaultKey = 0;
                defaultModifiers = Qt::NoModifier;
                break;
        }
        
        setHotkey(type, defaultKey, defaultModifiers);
    }
    
    // Window settings
    QJsonObject window;
    window["alwaysOnTop"] = true;
    window["opacity"] = 90;
    window["x"] = 100;
    window["y"] = 100;
    window["width"] = 800;
    window["height"] = 600;
    m_config["window"] = window;
    
    // General settings
    QJsonObject general;
    general["showWindowOnStartup"] = true;
    general["autoStartOnLogin"] = false;
    general["minimizeToTray"] = true;
    general["darkTheme"] = false;
    m_config["general"] = general;
    
    // Translation settings
    QJsonObject translation;
    translation["autoTranslate"] = false;
    m_config["translation"] = translation;
}

QString AppConfig::getHotkeyConfigKey(HotkeyType::Type type) const {
    switch (type) {
        case HotkeyType::MainToggle:
            return "hotkey";
        case HotkeyType::AlternativeToggle:
            return "alternativeToggleHotkey";
        default:
            return "unknown";
    }
}

// Generic hotkey methods
int AppConfig::getHotkeyKey(HotkeyType::Type type) const {
    QString configKey = getHotkeyConfigKey(type);
    QJsonObject hotkey = m_config[configKey].toObject();
    return hotkey["key"].toInt();
}

Qt::KeyboardModifiers AppConfig::getHotkeyModifiers(HotkeyType::Type type) const {
    QString configKey = getHotkeyConfigKey(type);
    QJsonObject hotkey = m_config[configKey].toObject();
    QJsonArray modifiersArray = hotkey["modifiers"].toArray();
    
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    for (const QJsonValue& value : modifiersArray) {
        modifiers |= static_cast<Qt::KeyboardModifier>(value.toInt());
    }
    return modifiers;
}

void AppConfig::setHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers) {
    QString configKey = getHotkeyConfigKey(type);
    
    QJsonObject hotkey;
    hotkey["key"] = key;
    
    QJsonArray modifiersArray;
    if (modifiers & Qt::ControlModifier)
        modifiersArray.append(static_cast<int>(Qt::ControlModifier));
    if (modifiers & Qt::AltModifier)
        modifiersArray.append(static_cast<int>(Qt::AltModifier));
    if (modifiers & Qt::ShiftModifier)
        modifiersArray.append(static_cast<int>(Qt::ShiftModifier));
    
    hotkey["modifiers"] = modifiersArray;
    m_config[configKey] = hotkey;
}

bool AppConfig::getAlwaysOnTop() const {
    return m_config["window"].toObject()["alwaysOnTop"].toBool(true);
}

void AppConfig::setAlwaysOnTop(bool value) {
    QJsonObject window = m_config["window"].toObject();
    window["alwaysOnTop"] = value;
    m_config["window"] = window;
}

int AppConfig::getWindowOpacity() const {
    return m_config["window"].toObject()["opacity"].toInt(90);
}

void AppConfig::setWindowOpacity(int value) {
    QJsonObject window = m_config["window"].toObject();
    window["opacity"] = qBound(20, value, 100);
    m_config["window"] = window;
}

int AppConfig::getWindowX() const {
    return m_config["window"].toObject()["x"].toInt(100);
}

void AppConfig::setWindowX(int value) {
    QJsonObject window = m_config["window"].toObject();
    window["x"] = value;
    m_config["window"] = window;
}

int AppConfig::getWindowY() const {
    return m_config["window"].toObject()["y"].toInt(100);
}

void AppConfig::setWindowY(int value) {
    QJsonObject window = m_config["window"].toObject();
    window["y"] = value;
    m_config["window"] = window;
}

int AppConfig::getWindowWidth() const {
    return m_config["window"].toObject()["width"].toInt(800);
}

void AppConfig::setWindowWidth(int value) {
    QJsonObject window = m_config["window"].toObject();
    window["width"] = qBound(400, value, 1920);
    m_config["window"] = window;
}

int AppConfig::getWindowHeight() const {
    return m_config["window"].toObject()["height"].toInt(600);
}

void AppConfig::setWindowHeight(int value) {
    QJsonObject window = m_config["window"].toObject();
    window["height"] = qBound(300, value, 1080);
    m_config["window"] = window;
}

bool AppConfig::getShowWindowOnStartup() const {
    return m_config["general"].toObject()["showWindowOnStartup"].toBool(true);
}

void AppConfig::setShowWindowOnStartup(bool value) {
    QJsonObject general = m_config["general"].toObject();
    general["showWindowOnStartup"] = value;
    m_config["general"] = general;
}

bool AppConfig::getAutoStartOnLogin() const {
    return m_config["general"].toObject()["autoStartOnLogin"].toBool(false);
}

void AppConfig::setAutoStartOnLogin(bool value) {
    QJsonObject general = m_config["general"].toObject();
    general["autoStartOnLogin"] = value;
    m_config["general"] = general;
    
#ifdef _WIN32
    // Update Windows registry autostart
    HKEY hKey;
    const wchar_t* regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* appName = L"TinyTools";
    
    if (value) {
        // Add to autostart
        if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            // Get application path
            wchar_t appPath[MAX_PATH];
            GetModuleFileNameW(NULL, appPath, MAX_PATH);
            
            // Set registry value
            RegSetValueExW(hKey, appName, 0, REG_SZ, (const BYTE*)appPath, (wcslen(appPath) + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);
            qInfo() << "Added application to Windows autostart registry";
        } else {
            qCritical() << "Failed to open registry key for autostart";
        }
    } else {
        // Remove from autostart
        if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegDeleteValueW(hKey, appName);
            RegCloseKey(hKey);
            qInfo() << "Removed application from Windows autostart registry";
        } else {
            qCritical() << "Failed to open registry key for autostart removal";
        }
    }
#endif
}

bool AppConfig::isAutoStartEnabledInRegistry() const {
#ifdef _WIN32
    HKEY hKey;
    const wchar_t* regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* appName = L"TinyTools";
    
    if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        wchar_t value[MAX_PATH];
        DWORD size = sizeof(value);
        DWORD type;
        
        bool exists = (RegQueryValueExW(hKey, appName, NULL, &type, (BYTE*)value, &size) == ERROR_SUCCESS);
        RegCloseKey(hKey);
        return exists;
    }
#endif
    return false;
}

bool AppConfig::getMinimizeToTray() const {
    return m_config["general"].toObject()["minimizeToTray"].toBool(true);
}

void AppConfig::setMinimizeToTray(bool value) {
    QJsonObject general = m_config["general"].toObject();
    general["minimizeToTray"] = value;
    m_config["general"] = general;
}

bool AppConfig::getDarkTheme() const {
    return m_config["general"].toObject()["darkTheme"].toBool(false);
}

void AppConfig::setDarkTheme(bool value) {
    QJsonObject general = m_config["general"].toObject();
    general["darkTheme"] = value;
    m_config["general"] = general;
    qInfo() << "Dark theme setting changed to:" << (value ? "enabled" : "disabled");
}

bool AppConfig::getAutoTranslate() const {
    return m_config["translation"].toObject()["autoTranslate"].toBool(false);
}

void AppConfig::setAutoTranslate(bool value) {
    QJsonObject translation = m_config["translation"].toObject();
    translation["autoTranslate"] = value;
    m_config["translation"] = translation;
    qInfo() << "Auto-translate setting changed to:" << (value ? "enabled" : "disabled");
}

QString AppConfig::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return configDir + "/settings.json";
}

QJsonObject AppConfig::getConfigObject() const {
    return m_config;
}

void AppConfig::setConfigObject(const QJsonObject& config) {
    m_config = config;
}

