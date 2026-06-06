#pragma once

#include <QJsonObject>
#include <QString>
#include <QUrl>
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
  QString id;       // UUID for unique identification
  QString name;     // Display name (e.g., "Google Translate")
  QString url;      // Resource URL
  QString iconPath; // Optional icon path (empty for default)

  // JavaScript automation scripts
  QString initScript;    // JS executed once on initial page load
  QString openScript;    // JS executed on normal open
  QString altOpenScript; // JS executed on alternative open

  bool isEnabled = true;   // Whether this resource is active
  int order = 0;           // Display order in tabs (0 = first)
  double zoomFactor = 1.0; // WebView zoom level (1.0 = 100%)

  /**
   * @brief Create a new WebResource with a generated UUID
   */
  static WebResource create(const QString &name, const QString &url) {
    WebResource resource;
    resource.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    resource.name = name;
    resource.url = url;
    resource.isEnabled = true;
    resource.order = 0;
    resource.zoomFactor = 1.0;
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
    obj["initScript"] = initScript;
    obj["openScript"] = openScript;
    obj["altOpenScript"] = altOpenScript;
    obj["isEnabled"] = isEnabled;
    obj["order"] = order;
    obj["zoomFactor"] = zoomFactor;
    return obj;
  }

  /**
   * @brief Deserialize from JSON
   */
  static WebResource fromJson(const QJsonObject &obj) {
    WebResource resource;
    resource.id = obj["id"].toString();
    resource.name = obj["name"].toString();
    resource.url = obj["url"].toString();
    resource.iconPath = obj["iconPath"].toString();
    resource.initScript = obj["initScript"].toString();
    resource.openScript = obj["openScript"].toString();
    resource.altOpenScript = obj["altOpenScript"].toString();
    resource.isEnabled = obj["isEnabled"].toBool(true);
    resource.order = obj["order"].toInt(0);
    resource.zoomFactor = obj["zoomFactor"].toDouble(1.0);

    // Generate ID if missing (for imported resources)
    if (resource.id.isEmpty()) {
      resource.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    return resource;
  }

  /**
   * @brief Check if this resource has a valid configuration.
   * Only http/https URLs are accepted to prevent file:// or javascript: injection.
   */
  bool isValid() const {
    if (id.isEmpty() || name.isEmpty() || url.isEmpty()) return false;
    QString scheme = QUrl(url).scheme().toLower();
    return scheme == QLatin1String("http") || scheme == QLatin1String("https");
  }

  /**
   * @brief Comparison for ordering in lists
   */
  bool operator<(const WebResource &other) const { return order < other.order; }
};
