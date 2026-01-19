#pragma once

#include <QString>
#include <QJsonObject>
#include <QUuid>
#include <Qt>

/**
 * @brief WebResource represents a single web resource configuration
 * 
 * Each resource has a URL, display name, optional icon, hotkeys for
 * opening (normal and alternative), and JavaScript scripts to execute
 * on page load for each open mode.
 */
struct WebResource {
    QString id;              // UUID for unique identification
    QString name;            // Display name (e.g., "Yandex Translate")
    QString url;             // Resource URL
    QString iconPath;        // Optional icon path (empty for default)
    
    // Normal open hotkey (per-resource, optional)
    int openHotkeyKey = 0;
    Qt::KeyboardModifiers openHotkeyModifiers = Qt::NoModifier;
    
    // Alternative open hotkey (per-resource, optional)  
    int altOpenHotkeyKey = 0;
    Qt::KeyboardModifiers altOpenHotkeyModifiers = Qt::NoModifier;
    
    // JavaScript automation scripts
    QString openScript;      // JS executed on normal open (e.g., focus input)
    QString altOpenScript;   // JS executed on alternative open (e.g., paste clipboard)
    
    bool isEnabled = true;   // Whether this resource is active
    int order = 0;           // Display order in tabs (0 = first)
    
    /**
     * @brief Create a new WebResource with a generated UUID
     */
    static WebResource create(const QString& name, const QString& url) {
        WebResource resource;
        resource.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        resource.name = name;
        resource.url = url;
        resource.isEnabled = true;
        resource.order = 0;
        return resource;
    }
    
    /**
     * @brief Serialize to JSON for storage
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["url"] = url;
        obj["iconPath"] = iconPath;
        obj["openHotkeyKey"] = openHotkeyKey;
        obj["openHotkeyModifiers"] = static_cast<int>(openHotkeyModifiers);
        obj["altOpenHotkeyKey"] = altOpenHotkeyKey;
        obj["altOpenHotkeyModifiers"] = static_cast<int>(altOpenHotkeyModifiers);
        obj["openScript"] = openScript;
        obj["altOpenScript"] = altOpenScript;
        obj["isEnabled"] = isEnabled;
        obj["order"] = order;
        return obj;
    }
    
    /**
     * @brief Deserialize from JSON
     */
    static WebResource fromJson(const QJsonObject& obj) {
        WebResource resource;
        resource.id = obj["id"].toString();
        resource.name = obj["name"].toString();
        resource.url = obj["url"].toString();
        resource.iconPath = obj["iconPath"].toString();
        resource.openHotkeyKey = obj["openHotkeyKey"].toInt(0);
        resource.openHotkeyModifiers = static_cast<Qt::KeyboardModifiers>(obj["openHotkeyModifiers"].toInt(0));
        resource.altOpenHotkeyKey = obj["altOpenHotkeyKey"].toInt(0);
        resource.altOpenHotkeyModifiers = static_cast<Qt::KeyboardModifiers>(obj["altOpenHotkeyModifiers"].toInt(0));
        resource.openScript = obj["openScript"].toString();
        resource.altOpenScript = obj["altOpenScript"].toString();
        resource.isEnabled = obj["isEnabled"].toBool(true);
        resource.order = obj["order"].toInt(0);
        
        // Generate ID if missing (for imported resources)
        if (resource.id.isEmpty()) {
            resource.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        
        return resource;
    }
    
    /**
     * @brief Check if this resource has a valid configuration
     */
    bool isValid() const {
        return !id.isEmpty() && !name.isEmpty() && !url.isEmpty();
    }
    
    /**
     * @brief Comparison for ordering in lists
     */
    bool operator<(const WebResource& other) const {
        return order < other.order;
    }
};
