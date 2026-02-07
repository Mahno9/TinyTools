#include "WebViewContainer.h"
#include "../models/ResourceManager.h"
#include <QChildEvent>
#include <QContextMenuEvent>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTimer>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWheelEvent>

// Custom QWebEnginePage to capture console logs
class LoggingWebEnginePage : public QWebEnginePage {
public:
  explicit LoggingWebEnginePage(QWebEngineProfile *profile,
                                QObject *parent = nullptr)
      : QWebEnginePage(profile, parent) {}

protected:
  void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                const QString &message, int lineNumber,
                                const QString &sourceID) override {
    QString levelStr;
    switch (level) {
    case InfoMessageLevel:
      levelStr = "INFO";
      break;
    case WarningMessageLevel:
      levelStr = "WARN";
      break;
    case ErrorMessageLevel:
      levelStr = "ERROR";
      break;
    default:
      levelStr = "LOG";
      break;
    }
    qDebug().noquote() << QString("[JS][%1] %2 (%3:%4)")
                              .arg(levelStr, message, sourceID,
                                   QString::number(lineNumber));
  }
};

// Static persistent profile shared across all WebViewContainers
static QWebEngineProfile *getPersistentProfile() {
  static QWebEngineProfile *profile = nullptr;
  if (!profile) {
    // Create persistent profile with storage path
    QString storagePath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    profile = new QWebEngineProfile("TinyTools", nullptr);
    profile->setPersistentStoragePath(storagePath + "/WebEngineData");
    profile->setCachePath(storagePath + "/WebEngineCache");
    profile->setPersistentCookiesPolicy(
        QWebEngineProfile::AllowPersistentCookies);
    qDebug() << "Created persistent WebEngine profile at:" << storagePath;
  }
  return profile;
}

WebViewContainer::WebViewContainer(QWidget *parent)
    : QWebEngineView(parent), m_darkThemeEnabled(false),
      m_darkThemeApplied(false) {
  // Configure page with persistent profile for cookie/auth persistence
  QWebEnginePage *page = new LoggingWebEnginePage(getPersistentProfile(), this);
  setPage(page);

  // Configure page settings for performance
  QWebEngineSettings *settings = page->settings();
  settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
  settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
  settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,
                         true);
  settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
  settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
  settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
  settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);
  // Keep this enabled just in case user scripts rely on it for other things,
  // though we are bypassing it for clipboard READ.
  settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,
                         true);

  // Connect signals
  connect(page, &QWebEnginePage::loadFinished, this,
          &WebViewContainer::onLoadFinished);
  connect(page, &QWebEnginePage::loadProgress, this,
          &WebViewContainer::onLoadProgress);
  connect(page, &QWebEnginePage::renderProcessTerminated, this,
          &WebViewContainer::onRenderProcessTerminated);

  // Install event filter on focus proxy to catch wheel events
  if (focusProxy()) {
    focusProxy()->installEventFilter(this);
  }
}

void WebViewContainer::loadResource(const WebResource &resource) {
  qDebug() << "WebViewContainer::loadResource() - ENTRY";
  qDebug() << "  Name:" << resource.name;
  qDebug() << "  URL String:" << resource.url;

  m_resourceId = resource.id;
  m_openScript = resource.openScript;
  m_altOpenScript = resource.altOpenScript;
  m_initScript = resource.initScript;

  // Apply zoom factor
  setZoomFactor(resource.zoomFactor);
  qDebug() << "  Zoom factor:" << resource.zoomFactor;

  QUrl url(resource.url);
  if (!url.isValid()) {
    qWarning() << "  WARNING: URL is invalid!";
  } else {
    qDebug() << "  Parsed QUrl scheme:" << url.scheme();
    qDebug() << "  Parsed QUrl host:" << url.host();
  }

  qInfo() << "Triggering load(QUrl)...";
  load(url);
  qDebug() << "WebViewContainer::loadResource() - EXIT";
}

void WebViewContainer::executeScript(const QString &script) {
  if (script.isEmpty())
    return;
  injectJavaScript(script);
}

void WebViewContainer::insertText(const QString &text) {
  if (isLoading()) {
    qWarning() << "Cannot execute open script: page is loading";
    return;
  }

  // Get fresh script from ResourceManager
  WebResource resource =
      ResourceManager::instance()->getResourceById(m_resourceId);
  QString openScript = resource.isValid() ? resource.openScript : m_openScript;

  if (openScript.isEmpty()) {
    qDebug() << "No open script defined for this resource";
    return;
  }

  qDebug() << "Executing open script with text length:" << text.length();

  // Safely escape the text by wrapping in a JSON array
  QJsonArray jsonArray;
  jsonArray.append(text);
  QString safeJson = QJsonDocument(jsonArray).toJson(QJsonDocument::Compact);

  // Inject the text into a global variable first
  QString code = QString("window.tinyToolsClipboard = %1[0];").arg(safeJson);

  // Execute variable injection, then user script
  // We use a lambda callback to ensure sequential execution
  page()->runJavaScript(code, [this, openScript](const QVariant &) {
    injectJavaScript(openScript);
  });
}

void WebViewContainer::insertAltText(const QString &text) {
  if (isLoading()) {
    qWarning() << "Cannot execute alt script: page is loading";
    return;
  }

  // Get fresh script from ResourceManager
  WebResource resource =
      ResourceManager::instance()->getResourceById(m_resourceId);
  QString altScript =
      resource.isValid() ? resource.altOpenScript : m_altOpenScript;

  if (altScript.isEmpty()) {
    qDebug() << "No alternative open script defined for this resource";
    return;
  }

  qDebug() << "Executing alternative open script with text length:"
           << text.length();

  // Same logic for Alt script
  QJsonArray jsonArray;
  jsonArray.append(text);
  QString safeJson = QJsonDocument(jsonArray).toJson(QJsonDocument::Compact);
  QString code = QString("window.tinyToolsClipboard = %1[0];").arg(safeJson);

  page()->runJavaScript(code, [this, altScript](const QVariant &) {
    injectJavaScript(altScript);
  });
}

void WebViewContainer::reloadTranslator() { reload(); }

bool WebViewContainer::isLoading() const {
  return page() && page()->isLoading();
}

void WebViewContainer::onLoadFinished(bool ok) {
  if (ok) {
    qDebug() << "Page loaded successfully";
    emit pageLoaded(true);

    // Apply dark theme preference to the page
    applyWebViewTheme(m_darkThemeEnabled);

    if (!m_initScript.isEmpty()) {
      qDebug() << "Executing initialization script...";
      injectJavaScript(m_initScript);
    }
  } else {
    qWarning() << "Page load failed";
    emit loadError("Failed to load resource page");
    emit pageLoaded(false);
  }
}

void WebViewContainer::onLoadProgress(int progress) {
  if (progress % 25 == 0) {
    qDebug() << "Loading progress:" << progress << "%";
  }
}

void WebViewContainer::onRenderProcessTerminated(
    QWebEnginePage::RenderProcessTerminationStatus status, int exitCode) {
  Q_UNUSED(exitCode);

  QString reason;
  switch (status) {
  case QWebEnginePage::NormalTerminationStatus:
    reason = "Normal termination";
    break;
  case QWebEnginePage::AbnormalTerminationStatus:
    reason = "Abnormal termination";
    break;
  case QWebEnginePage::CrashedTerminationStatus:
    reason = "Render process crashed";
    break;
  case QWebEnginePage::KilledTerminationStatus:
    reason = "Render process killed";
    break;
  }

  qCritical() << "Render process terminated:" << reason;
  emit loadError(reason);

  // Attempt to reload
  QTimer::singleShot(1000, this, &WebViewContainer::reload);
}

void WebViewContainer::injectJavaScript(const QString &script) {
  if (!page())
    return;

  page()->runJavaScript(script, [this](const QVariant &result) {
    // Optional: handle result
  });
}

void WebViewContainer::waitForPageLoad() {
  // No-op
}

void WebViewContainer::contextMenuEvent(QContextMenuEvent *event) {
  // Disable context menu for cleaner UI
  event->ignore();
}

void WebViewContainer::applyWebViewTheme(bool darkTheme) {
  m_darkThemeEnabled = darkTheme;

  if (!page())
    return;

  // Remove any existing theme script
  QWebEngineScriptCollection &scripts = page()->scripts();
  QList<QWebEngineScript> existingScripts =
      scripts.find("__colorSchemeOverride");
  for (const QWebEngineScript &s : existingScripts) {
    scripts.remove(s);
  }

  // Create JavaScript that overrides matchMedia BEFORE any page JS runs
  QString scriptSource = QString(R"(
    (function() {
      const isDark = %1;
      const colorScheme = isDark ? 'dark' : 'light';
      
      // Override matchMedia for prefers-color-scheme queries
      const originalMatchMedia = window.matchMedia.bind(window);
      Object.defineProperty(window, 'matchMedia', {
        value: function(query) {
          if (query === '(prefers-color-scheme: dark)') {
            return {
              matches: isDark,
              media: query,
              onchange: null,
              addListener: function(cb) { /* deprecated */ },
              removeListener: function(cb) { /* deprecated */ },
              addEventListener: function(type, cb) {},
              removeEventListener: function(type, cb) {},
              dispatchEvent: function(e) { return true; }
            };
          } else if (query === '(prefers-color-scheme: light)') {
            return {
              matches: !isDark,
              media: query,
              onchange: null,
              addListener: function(cb) {},
              removeListener: function(cb) {},
              addEventListener: function(type, cb) {},
              removeEventListener: function(type, cb) {},
              dispatchEvent: function(e) { return true; }
            };
          }
          return originalMatchMedia(query);
        },
        writable: false,
        configurable: false
      });
      
      console.log('[TinyTools] Color scheme preference applied:', colorScheme);
    })();
  )")
                             .arg(darkTheme ? "true" : "false");

  // Create user script that runs at document creation (BEFORE page JS)
  QWebEngineScript script;
  script.setName("__colorSchemeOverride");
  script.setSourceCode(scriptSource);
  script.setInjectionPoint(QWebEngineScript::DocumentCreation);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(true);

  scripts.insert(script);

  qDebug() << "WebView theme script installed, dark mode:" << darkTheme;
}

bool WebViewContainer::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::Wheel) {
    QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
    if (wheelEvent->modifiers() & Qt::ControlModifier) {
      const double step = 0.1; // 10% per scroll step
      double currentZoom = zoomFactor();

      if (wheelEvent->angleDelta().y() > 0) {
        // Scroll up - zoom in
        currentZoom = qMin(currentZoom + step, 3.0);
      } else {
        // Scroll down - zoom out
        currentZoom = qMax(currentZoom - step, 0.3);
      }

      setZoomFactor(currentZoom);
      emit zoomChanged(m_resourceId, currentZoom);
      return true; // Event handled
    }
  }
  return QWebEngineView::eventFilter(obj, event);
}

void WebViewContainer::childEvent(QChildEvent *event) {
  QWebEngineView::childEvent(event);
  // When child is added (render widget), install event filter
  if (event->added() && focusProxy()) {
    focusProxy()->installEventFilter(this);
  }
}
