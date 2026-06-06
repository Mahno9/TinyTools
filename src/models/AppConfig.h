#pragma once
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <QObject>
#include <QMutex>
#include <Qt>

// Forward declaration of HotkeyType namespace
namespace HotkeyType {
    enum Type;
}

class AppConfig : public QObject {
    Q_OBJECT

public:
    static AppConfig* instance();
    static void cleanupInstance();

    bool load();
    bool save();
    void resetToDefaults();
    QString getConfigFilePath() const;
    
    // Consolidated hotkey methods
    int getHotkeyKey(HotkeyType::Type type) const;
    Qt::KeyboardModifiers getHotkeyModifiers(HotkeyType::Type type) const;
    void setHotkey(HotkeyType::Type type, int key, Qt::KeyboardModifiers modifiers);
    
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
    bool getShowWindowOnStartup() const;
    void setShowWindowOnStartup(bool value);
    
    bool getAutoStartOnLogin() const;
    void setAutoStartOnLogin(bool value);
    bool isAutoStartEnabledInRegistry() const;
    
    bool getMinimizeToTray() const;
    void setMinimizeToTray(bool value);
    
    bool getDarkTheme() const;
    void setDarkTheme(bool value);
    
    // Translation settings
    bool getAutoTranslate() const;
    void setAutoTranslate(bool value);
    
    // Raw config access (for ResourceManager)
    QJsonObject getConfigObject() const;
    void setConfigObject(const QJsonObject& config);
    
signals:
    void settingsChanged();
    
private:
    AppConfig(); // Private constructor for singleton pattern
    QString getHotkeyConfigKey(HotkeyType::Type type) const;
    
    static QPointer<AppConfig> s_instance;
    static QMutex s_mutex;
    
    QJsonObject m_config;
    QString m_configPath;
    
    Q_DISABLE_COPY(AppConfig)
};
