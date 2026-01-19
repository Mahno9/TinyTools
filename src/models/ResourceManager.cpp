#include "ResourceManager.h"
#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMutexLocker>
#include <algorithm>

// Static instance
QPointer<ResourceManager> ResourceManager::s_instance = nullptr;
QMutex ResourceManager::s_mutex;

ResourceManager* ResourceManager::instance() {
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return s_instance;
}

ResourceManager::ResourceManager()
    : QObject(nullptr)
    , m_startupMode(LastUsed)
{
    qDebug() << "ResourceManager created";
}

QList<WebResource> ResourceManager::getAllResources() const {
    return m_resources;
}

WebResource ResourceManager::getResourceById(const QString& id) const {
    for (const auto& resource : m_resources) {
        if (resource.id == id) {
            return resource;
        }
    }
    return WebResource();
}

WebResource ResourceManager::getResourceByIndex(int index) const {
    if (index >= 0 && index < m_resources.size()) {
        return m_resources.at(index);
    }
    return WebResource();
}

int ResourceManager::getResourceCount() const {
    return m_resources.size();
}

int ResourceManager::getResourceIndex(const QString& id) const {
    for (int i = 0; i < m_resources.size(); ++i) {
        if (m_resources.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

void ResourceManager::addResource(const WebResource& resource) {
    if (!resource.isValid()) {
        qWarning() << "Cannot add invalid resource";
        return;
    }
    
    // Check for duplicate ID
    for (const auto& existing : m_resources) {
        if (existing.id == resource.id) {
            qWarning() << "Resource with ID already exists:" << resource.id;
            return;
        }
    }
    
    m_resources.append(resource);
    qDebug() << "Added resource:" << resource.name << "with ID:" << resource.id;
    
    emit resourceAdded(resource.id);
    emit resourcesChanged();
}

void ResourceManager::removeResource(const QString& id) {
    for (int i = 0; i < m_resources.size(); ++i) {
        if (m_resources.at(i).id == id) {
            QString name = m_resources.at(i).name;
            m_resources.removeAt(i);
            qDebug() << "Removed resource:" << name << "with ID:" << id;
            
            // Update default/last used if removed
            if (m_defaultResourceId == id) {
                m_defaultResourceId.clear();
            }
            if (m_lastUsedResourceId == id) {
                m_lastUsedResourceId.clear();
            }
            
            emit resourceRemoved(id);
            emit resourcesChanged();
            return;
        }
    }
    qWarning() << "Resource not found for removal:" << id;
}

void ResourceManager::updateResource(const WebResource& resource) {
    for (int i = 0; i < m_resources.size(); ++i) {
        if (m_resources.at(i).id == resource.id) {
            m_resources[i] = resource;
            qDebug() << "Updated resource:" << resource.name;
            
            emit resourceUpdated(resource.id);
            emit resourcesChanged();
            return;
        }
    }
    qWarning() << "Resource not found for update:" << resource.id;
}

void ResourceManager::reorderResources(const QStringList& orderedIds) {
    QList<WebResource> reordered;
    int order = 0;
    
    for (const QString& id : orderedIds) {
        for (auto& resource : m_resources) {
            if (resource.id == id) {
                resource.order = order++;
                reordered.append(resource);
                break;
            }
        }
    }
    
    // Add any resources not in orderedIds at the end
    for (auto& resource : m_resources) {
        bool found = false;
        for (const auto& reorderedRes : reordered) {
            if (reorderedRes.id == resource.id) {
                found = true;
                break;
            }
        }
        if (!found) {
            resource.order = order++;
            reordered.append(resource);
        }
    }
    
    m_resources = reordered;
    emit resourcesChanged();
}

void ResourceManager::clearAllResources() {
    m_resources.clear();
    m_defaultResourceId.clear();
    m_lastUsedResourceId.clear();
    emit resourcesChanged();
}

ResourceManager::StartupMode ResourceManager::getStartupMode() const {
    return m_startupMode;
}

void ResourceManager::setStartupMode(StartupMode mode) {
    if (m_startupMode != mode) {
        m_startupMode = mode;
        emit startupModeChanged(mode);
    }
}

QString ResourceManager::getDefaultResourceId() const {
    return m_defaultResourceId;
}

void ResourceManager::setDefaultResourceId(const QString& id) {
    m_defaultResourceId = id;
}

QString ResourceManager::getLastUsedResourceId() const {
    return m_lastUsedResourceId;
}

void ResourceManager::setLastUsedResourceId(const QString& id) {
    if (m_lastUsedResourceId != id) {
        m_lastUsedResourceId = id;
        emit activeResourceChanged(id);
    }
}

WebResource ResourceManager::getStartupResource() const {
    QString targetId;
    
    switch (m_startupMode) {
        case LastUsed:
            targetId = m_lastUsedResourceId;
            break;
        case SelectedResource:
            targetId = m_defaultResourceId;
            break;
    }
    
    // Try to get the target resource
    WebResource resource = getResourceById(targetId);
    
    // Fallback to first resource if target not found
    if (!resource.isValid() && !m_resources.isEmpty()) {
        resource = m_resources.first();
    }
    
    return resource;
}

bool ResourceManager::importPresets(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for import:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error during import:" << error.errorString();
        return false;
    }
    
    QJsonArray resourcesArray = doc.object()["resources"].toArray();
    int importedCount = 0;
    
    for (const QJsonValue& value : resourcesArray) {
        WebResource resource = WebResource::fromJson(value.toObject());
        
        // Generate new ID to avoid conflicts (append mode)
        resource.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        resource.order = m_resources.size(); // Add at end
        
        if (resource.isValid()) {
            m_resources.append(resource);
            emit resourceAdded(resource.id);
            importedCount++;
        }
    }
    
    if (importedCount > 0) {
        emit resourcesChanged();
        qInfo() << "Imported" << importedCount << "resources from" << filePath;
    }
    
    return importedCount > 0;
}

bool ResourceManager::exportPresets(const QString& filePath) const {
    QJsonArray resourcesArray;
    
    for (const auto& resource : m_resources) {
        resourcesArray.append(resource.toJson());
    }
    
    QJsonObject root;
    root["version"] = "1.0";
    root["resources"] = resourcesArray;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for export:" << filePath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qInfo() << "Exported" << m_resources.size() << "resources to" << filePath;
    return true;
}

bool ResourceManager::loadFromConfig() {
    qDebug() << "ResourceManager::loadFromConfig() - ENTRY";
    AppConfig* config = AppConfig::instance();
    QJsonObject configObj = config->getConfigObject();
    
    // Load resources
    QJsonArray resourcesArray = configObj["resources"].toArray();
    qDebug() << "Found" << resourcesArray.size() << "resource resources in config JSON";
    
    m_resources.clear();
    
    for (int i = 0; i < resourcesArray.size(); ++i) {
        QJsonValue value = resourcesArray.at(i);
        QJsonObject resObj = value.toObject();
        WebResource resource = WebResource::fromJson(resObj);
        
        qDebug() << "Loading resource candidate:" << i 
                 << "ID:" << resource.id 
                 << "Name:" << resource.name
                 << "URL:" << resource.url;
                 
        if (resource.isValid()) {
            m_resources.append(resource);
            qDebug() << "Resource loaded successfully";
        } else {
            qWarning() << "Skipping invalid resource at index" << i;
        }
    }
    
    // Sort by order
    std::sort(m_resources.begin(), m_resources.end());
    
    // Load startup settings
    QJsonObject startup = configObj["startup"].toObject();
    QString modeStr = startup["mode"].toString("lastUsed");
    m_startupMode = (modeStr == "selected") ? SelectedResource : LastUsed;
    m_defaultResourceId = startup["defaultResourceId"].toString();
    m_lastUsedResourceId = startup["lastUsedResourceId"].toString();
    
    qInfo() << "Loaded" << m_resources.size() << "resources from config";
    qDebug() << "ResourceManager::loadFromConfig() - EXIT";
    return true;
}

bool ResourceManager::saveToConfig() {
    qDebug() << "ResourceManager::saveToConfig() - ENTRY";
    AppConfig* config = AppConfig::instance();
    QJsonObject configObj = config->getConfigObject();
    
    // Save resources
    QJsonArray resourcesArray;
    qDebug() << "Serializing" << m_resources.size() << "resources...";
    
    for (const auto& resource : m_resources) {
        if (resource.isValid()) {
            resourcesArray.append(resource.toJson());
            qDebug() << "Serialized resource:" << resource.name << "(" << resource.id << ")";
        } else {
            qWarning() << "Skipping invalid resource during save:" << resource.id;
        }
    }
    configObj["resources"] = resourcesArray;
    qDebug() << "Updated config object with" << resourcesArray.size() << "resources";
    
    // Save startup settings
    QJsonObject startup;
    startup["mode"] = (m_startupMode == SelectedResource) ? "selected" : "lastUsed";
    startup["defaultResourceId"] = m_defaultResourceId;
    startup["lastUsedResourceId"] = m_lastUsedResourceId;
    configObj["startup"] = startup;
    
    config->setConfigObject(configObj);
    
    qDebug() << "Triggering AppConfig::save()...";
    bool result = config->save();
    qDebug() << "AppConfig::save() returned:" << result;
    
    qDebug() << "ResourceManager::saveToConfig() - EXIT";
    return result;
}
