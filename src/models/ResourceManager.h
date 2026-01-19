#pragma once

#include <QObject>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QMutex>
#include "WebResource.h"

/**
 * @brief ResourceManager is a singleton that manages all web resources
 * 
 * Provides CRUD operations for resources, import/export functionality,
 * and startup behavior settings.
 */
class ResourceManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Startup mode determines which resource opens on app start
     */
    enum StartupMode {
        LastUsed,           // Open the last used resource
        SelectedResource    // Always open the configured default resource
    };
    Q_ENUM(StartupMode)
    
    /**
     * @brief Get the singleton instance
     */
    static ResourceManager* instance();
    
    // Resource CRUD operations
    QList<WebResource> getAllResources() const;
    WebResource getResourceById(const QString& id) const;
    WebResource getResourceByIndex(int index) const;
    int getResourceCount() const;
    int getResourceIndex(const QString& id) const;
    
    void addResource(const WebResource& resource);
    void removeResource(const QString& id);
    void updateResource(const WebResource& resource);
    void reorderResources(const QStringList& orderedIds);
    void clearAllResources();
    
    // Startup behavior
    StartupMode getStartupMode() const;
    void setStartupMode(StartupMode mode);
    
    QString getDefaultResourceId() const;
    void setDefaultResourceId(const QString& id);
    
    QString getLastUsedResourceId() const;
    void setLastUsedResourceId(const QString& id);
    
    /**
     * @brief Get the resource to open on startup based on current settings
     */
    WebResource getStartupResource() const;
    
    // Preset import/export (append mode for import)
    bool importPresets(const QString& filePath);
    bool exportPresets(const QString& filePath) const;
    
    // Persistence
    bool loadFromConfig();
    bool saveToConfig();
    
signals:
    void resourcesChanged();
    void resourceAdded(const QString& resourceId);
    void resourceRemoved(const QString& resourceId);
    void resourceUpdated(const QString& resourceId);
    void activeResourceChanged(const QString& resourceId);
    void startupModeChanged(StartupMode mode);

private:
    ResourceManager();
    ~ResourceManager() = default;
    
    static QPointer<ResourceManager> s_instance;
    static QMutex s_mutex;
    
    QList<WebResource> m_resources;
    StartupMode m_startupMode = LastUsed;
    QString m_defaultResourceId;
    QString m_lastUsedResourceId;
    
    Q_DISABLE_COPY(ResourceManager)
};
