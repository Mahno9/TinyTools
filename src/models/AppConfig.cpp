#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

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
    QJsonDocument doc(m_config);
    
    // Ensure directory exists
    QDir dir = QFileInfo(m_configPath).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Cannot create config directory:" << dir.path();
            return false;
        }
    }
    
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open config file for writing:" << m_configPath;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qInfo() << "Configuration saved to:" << m_configPath;
    return true;
}

void AppConfig::resetToDefaults() {
    // Hotkey: Ctrl+Alt+T
    QJsonObject hotkey;
    hotkey["key"] = static_cast<int>(Qt::Key_T);
    QJsonArray modifiers;
    modifiers.append(static_cast<int>(Qt::ControlModifier));
    modifiers.append(static_cast<int>(Qt::AltModifier));
    hotkey["modifiers"] = modifiers;
    m_config["hotkey"] = hotkey;
    
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
    general["autoStart"] = true;
    general["minimizeToTray"] = true;
    general["language"] = "en";
    m_config["general"] = general;
    
    // Translation settings
    QJsonObject translation;
    translation["autoTranslate"] = false;
    translation["sourceLanguage"] = "auto";
    translation["targetLanguage"] = "en";
    m_config["translation"] = translation;
}

int AppConfig::getHotkeyKey() const {
    QJsonObject hotkey = m_config["hotkey"].toObject();
    return hotkey["key"].toInt();
}

Qt::KeyboardModifiers AppConfig::getHotkeyModifiers() const {
    QJsonObject hotkey = m_config["hotkey"].toObject();
    QJsonArray modifiersArray = hotkey["modifiers"].toArray();
    
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    for (const QJsonValue& value : modifiersArray) {
        modifiers |= static_cast<Qt::KeyboardModifier>(value.toInt());
    }
    
    return modifiers;
}

void AppConfig::setHotkey(int key, Qt::KeyboardModifiers modifiers) {
    QJsonObject hotkey;
    hotkey["key"] = key;
    
    QJsonArray modifiersArray;
    if (modifiers & Qt::ControlModifier)
        modifiersArray.append(static_cast<int>(Qt::ControlModifier));
    if (modifiers & Qt::AltModifier)
        modifiersArray.append(static_cast<int>(Qt::AltModifier));
    if (modifiers & Qt::ShiftModifier)
        modifiersArray.append(static_cast<int>(Qt::ShiftModifier));
    if (modifiers & Qt::MetaModifier)
        modifiersArray.append(static_cast<int>(Qt::MetaModifier));
    
    hotkey["modifiers"] = modifiersArray;
    m_config["hotkey"] = hotkey;
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

bool AppConfig::getAutoStart() const {
    return m_config["general"].toObject()["autoStart"].toBool(true);
}

void AppConfig::setAutoStart(bool value) {
    QJsonObject general = m_config["general"].toObject();
    general["autoStart"] = value;
    m_config["general"] = general;
}

bool AppConfig::getMinimizeToTray() const {
    return m_config["general"].toObject()["minimizeToTray"].toBool(true);
}

void AppConfig::setMinimizeToTray(bool value) {
    QJsonObject general = m_config["general"].toObject();
    general["minimizeToTray"] = value;
    m_config["general"] = general;
}

QString AppConfig::getLanguage() const {
    return m_config["general"].toObject()["language"].toString("en");
}

void AppConfig::setLanguage(const QString& value) {
    QJsonObject general = m_config["general"].toObject();
    general["language"] = value;
    m_config["general"] = general;
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

QString AppConfig::getSourceLanguage() const {
    return m_config["translation"].toObject()["sourceLanguage"].toString("auto");
}

void AppConfig::setSourceLanguage(const QString& value) {
    QJsonObject translation = m_config["translation"].toObject();
    translation["sourceLanguage"] = value;
    m_config["translation"] = translation;
    qInfo() << "Source language changed to:" << value;
}

QString AppConfig::getTargetLanguage() const {
    return m_config["translation"].toObject()["targetLanguage"].toString("en");
}

void AppConfig::setTargetLanguage(const QString& value) {
    QJsonObject translation = m_config["translation"].toObject();
    translation["targetLanguage"] = value;
    m_config["translation"] = translation;
    qInfo() << "Target language changed to:" << value;
}

QString AppConfig::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return configDir + "/settings.json";
}
