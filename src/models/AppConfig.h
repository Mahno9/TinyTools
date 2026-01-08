#pragma once
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <QObject>
#include <QMutex>

class AppConfig : public QObject {
    Q_OBJECT

public:
    static AppConfig* instance();
    
    bool load();
    bool save();
    void resetToDefaults();
    QString getConfigFilePath() const;
    
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
    
    bool getDarkTheme() const;
    void setDarkTheme(bool value);
    
    // Translation settings
    bool getAutoTranslate() const;
    void setAutoTranslate(bool value);
    
signals:
    void settingsChanged();
    
private:
    AppConfig(); // Private constructor for singleton pattern
    
    static QPointer<AppConfig> s_instance;
    static QMutex s_mutex;
    
    QJsonObject m_config;
    QString m_configPath;
    
    Q_DISABLE_COPY(AppConfig)
};
