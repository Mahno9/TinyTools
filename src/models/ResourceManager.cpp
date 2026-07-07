#include "ResourceManager.h"
#include "AppConfig.h"
#include "../app/Constants.h"
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

void ResourceManager::cleanupInstance() {
    QMutexLocker locker(&s_mutex);
    delete s_instance.data();
    s_instance = nullptr;
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

void ResourceManager::setResourceZoom(const QString& id, double zoomFactor) {
    for (auto& resource : m_resources) {
        if (resource.id == id) {
            resource.zoomFactor = zoomFactor;
            return;
        }
    }
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

QList<WebResource> ResourceManager::parsePresets(const QString& filePath,
                                                 QString* errorOut) {
    QList<WebResource> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorOut) *errorOut = QStringLiteral("Cannot open file: %1").arg(filePath);
        return result;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError) {
        if (errorOut) *errorOut = QStringLiteral("JSON parse error: %1").arg(error.errorString());
        return result;
    }

    const QJsonArray resourcesArray = doc.object()["resources"].toArray();
    for (const QJsonValue& value : resourcesArray) {
        WebResource resource = WebResource::fromJson(value.toObject());

        // Always regenerate the ID: presets may collide with existing
        // resources or contain duplicate ids within the file.
        resource.id = QUuid::createUuid().toString(QUuid::WithoutBraces);

        // isValid() enforces http/https scheme with a host - rejects
        // file:// and javascript: URLs.
        if (resource.isValid()) {
            result.append(resource);
        } else {
            qWarning() << "Skipping imported resource with invalid URL:" << resource.url;
        }
    }
    return result;
}

bool ResourceManager::writePresets(const QString& filePath,
                                   const QList<WebResource>& resources) {
    QJsonArray resourcesArray;
    for (const auto& resource : resources) {
        resourcesArray.append(resource.toJson());
    }

    QJsonObject root;
    root["version"] = "1.0";
    root["resources"] = resourcesArray;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for export:" << filePath;
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "Exported" << resources.size() << "resources to" << filePath;
    return true;
}

bool ResourceManager::importPresets(const QString& filePath) {
    QString error;
    const QList<WebResource> parsed = parsePresets(filePath, &error);
    if (!error.isEmpty()) {
        qWarning() << "Import failed:" << error;
        return false;
    }

    int importedCount = 0;
    for (WebResource resource : parsed) {
        if (m_resources.size() >= Constants::MAX_RESOURCES) {
            qWarning() << "Import cap reached at" << Constants::MAX_RESOURCES
                       << "resources - skipping remainder";
            break;
        }
        resource.order = m_resources.size(); // Add at end
        m_resources.append(resource);
        emit resourceAdded(resource.id);
        importedCount++;
    }

    if (importedCount > 0) {
        emit resourcesChanged();
        qInfo() << "Imported" << importedCount << "resources from" << filePath;
    }

    return importedCount > 0;
}

bool ResourceManager::exportPresets(const QString& filePath) const {
    return writePresets(filePath, m_resources);
}

bool ResourceManager::loadFromConfig() {
    AppConfig* config = AppConfig::instance();
    QJsonObject configObj = config->getConfigObject();

    QJsonArray resourcesArray = configObj["resources"].toArray();
    m_resources.clear();

    for (int i = 0; i < resourcesArray.size(); ++i) {
        WebResource resource = WebResource::fromJson(resourcesArray.at(i).toObject());
        if (resource.isValid()) {
            m_resources.append(resource);
        } else {
            qWarning() << "Skipping invalid resource at index" << i
                       << "URL:" << resource.url;
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
    return true;
}

bool ResourceManager::saveToConfig() {
    AppConfig* config = AppConfig::instance();
    QJsonObject configObj = config->getConfigObject();

    QJsonArray resourcesArray;
    for (const auto& resource : m_resources) {
        if (resource.isValid()) {
            resourcesArray.append(resource.toJson());
        } else {
            qWarning() << "Skipping invalid resource during save:" << resource.id;
        }
    }
    configObj["resources"] = resourcesArray;

    QJsonObject startup;
    startup["mode"] = (m_startupMode == SelectedResource) ? "selected" : "lastUsed";
    startup["defaultResourceId"] = m_defaultResourceId;
    startup["lastUsedResourceId"] = m_lastUsedResourceId;
    configObj["startup"] = startup;

    config->setConfigObject(configObj);
    return config->save();
}
