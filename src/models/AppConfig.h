#pragma once
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class AppConfig {
public:
    AppConfig();
    
    bool load();
    bool save();
    void resetToDefaults();
    
    // Hotkey settings
    int getHotkeyKey() const;
    Qt::KeyboardModifiers getHotkeyModifiers() const;
    void setHotkey(int key, Qt::KeyboardModifiers modifiers);
    
    // Window settings
    bool getAlwaysOnTop() const;
    void setAlwaysOnTop(bool value);
    
    int getWindowOpacity() const; // 0-100
    void setWindowOpacity(int value);
    
    int getWindowX() const;
    void setWindowX(int value);
    
    int getWindowY() const;
    void setWindowY(int value);
    
    int getWindowWidth() const;
    void setWindowWidth(int value);
    
    int getWindowHeight() const;
    void setWindowHeight(int value);
    
    // General settings
    bool getAutoStart() const;
    void setAutoStart(bool value);
    
    bool getMinimizeToTray() const;
    void setMinimizeToTray(bool value);
    
    QString getLanguage() const;
    void setLanguage(const QString& value);
    
private:
    QString getConfigFilePath() const;
    
    QJsonObject m_config;
    QString m_configPath;
};
