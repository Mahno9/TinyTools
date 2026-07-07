#include "WebViewContainer.h"
#include "../app/Logging.h"
#include "../models/ResourceManager.h"
#include <QApplication>
#include <QChildEvent>
#include <QContextMenuEvent>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTimer>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWheelEvent>

namespace {
constexpr int MAX_CRASH_RELOADS = 3;

// Custom QWebEnginePage to capture console logs (debug builds/runs only -
// page console output can contain page data and must not land in user logs).
class LoggingWebEnginePage : public QWebEnginePage {
public:
  explicit LoggingWebEnginePage(QWebEngineProfile *profile,
                                QObject *parent = nullptr)
      : QWebEnginePage(profile, parent) {}

protected:
  void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                const QString &message, int lineNumber,
                                const QString &sourceID) override {
    if (!Logging::isDebugEnabled())
      return;
    qDebug().noquote() << QString("[JS][%1] %2 (%3:%4)")
                              .arg(int(level))
                              .arg(message, sourceID,
                                   QString::number(lineNumber));
  }
};

// Static persistent profile shared across all WebViewContainers
QWebEngineProfile *getPersistentProfile() {
  static QWebEngineProfile *profile = nullptr;
  if (!profile) {
    QString storagePath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // Parent to qApp so Qt destroys the profile when QApplication exits.
    // MainWindow (and its WebViewContainers/pages) is deleted by
    // Application::~Application() before QApplication runs its children
    // cleanup, ensuring correct destruction order.
    profile = new QWebEngineProfile("TinyTools", qApp);
    profile->setPersistentStoragePath(storagePath + "/WebEngineData");
    profile->setCachePath(storagePath + "/WebEngineCache");
    profile->setPersistentCookiesPolicy(
        QWebEngineProfile::AllowPersistentCookies);
  }
  return profile;
}
} // namespace

WebViewContainer::WebViewContainer(QWidget *parent) : QWebEngineView(parent) {
  // Configure page with persistent profile for cookie/auth persistence
  QWebEnginePage *page = new LoggingWebEnginePage(getPersistentProfile(), this);
  setPage(page);

  QWebEngineSettings *settings = page->settings();
  settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
  settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
  settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,
                         true);
  settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
  settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
  settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
  settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);
  // Disabled: web pages must not read the OS clipboard directly.
  // Text is injected via window.tinyToolsClipboard global instead.
  settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,
                         false);

  connect(page, &QWebEnginePage::loadFinished, this,
          &WebViewContainer::onLoadFinished);
  connect(page, &QWebEnginePage::renderProcessTerminated, this,
          &WebViewContainer::onRenderProcessTerminated);

  // Install event filter on focus proxy to catch wheel events
  if (focusProxy()) {
    focusProxy()->installEventFilter(this);
  }
}

void WebViewContainer::loadResource(const WebResource &resource) {
  m_resourceId = resource.id;
  setZoomFactor(resource.zoomFactor);

  if (!resource.isValid()) {
    qWarning() << "Refusing to load invalid resource:" << resource.url;
    m_resourceUrl.clear();
    showErrorPage(tr("Invalid URL: %1").arg(resource.url));
    return;
  }

  m_resourceUrl = QUrl(resource.url);
  m_hasLoadedOk = false;
  m_crashCount = 0;

  qInfo() << "Loading resource:" << resource.name << m_resourceUrl.toString();
  load(m_resourceUrl);
}

void WebViewContainer::insertText(const QString &text) {
  runOpenScript(false, text);
}

void WebViewContainer::insertAltText(const QString &text) {
  runOpenScript(true, text);
}

void WebViewContainer::runOpenScript(bool alt, const QString &text) {
  if (isLoading()) {
    qWarning() << "Cannot execute open script: page is loading";
    return;
  }

  // Always fetch the current script from ResourceManager so edits in the
  // settings dialog take effect without recreating the view.
  WebResource resource =
      ResourceManager::instance()->getResourceById(m_resourceId);
  if (!resource.isValid()) {
    return;
  }
  const QString script = alt ? resource.altOpenScript : resource.openScript;
  if (script.isEmpty()) {
    return;
  }

  // Safely escape the text by wrapping in a JSON array
  QJsonArray jsonArray;
  jsonArray.append(text);
  QString safeJson = QJsonDocument(jsonArray).toJson(QJsonDocument::Compact);
  QString code = QString("window.tinyToolsClipboard = %1[0];").arg(safeJson);

  // Inject the clipboard global, run the user script, then schedule cleanup
  // so page scripts cannot read the injected text indefinitely. The delay
  // gives async user scripts (waiting for elements etc.) a grace period.
  page()->runJavaScript(code, [this, script](const QVariant &) {
    page()->runJavaScript(script, [this](const QVariant &) {
      page()->runJavaScript(
          "setTimeout(function(){"
          " try { delete window.tinyToolsClipboard; } catch(e) {}"
          "}, 5000);");
    });
  });
}

void WebViewContainer::reloadPage() {
  if (m_resourceUrl.isValid() && url().scheme() != QLatin1String("http") &&
      url().scheme() != QLatin1String("https")) {
    // Currently showing an internal error page: navigate back to the resource.
    load(m_resourceUrl);
  } else {
    reload();
  }
}

bool WebViewContainer::isLoading() const {
  return page() && page()->isLoading();
}

void WebViewContainer::onLoadFinished(bool ok) {
  const bool isResourcePage = url().scheme() == QLatin1String("http") ||
                              url().scheme() == QLatin1String("https");

  if (ok) {
    emit pageLoaded(true);
    if (!isResourcePage) {
      return; // Internal error page: no theme/init script needed.
    }
    m_hasLoadedOk = true;
    m_crashCount = 0;

    // Apply dark theme preference to the page
    applyWebViewTheme(m_darkThemeEnabled);

    WebResource resource =
        ResourceManager::instance()->getResourceById(m_resourceId);
    if (resource.isValid() && !resource.initScript.isEmpty()) {
      page()->runJavaScript(resource.initScript);
    }
  } else {
    qWarning() << "Page load failed:" << m_resourceUrl.toString();
    emit pageLoaded(false);
    // loadFinished(false) also fires for aborted navigations on an already
    // rendered page; only replace content that never loaded successfully.
    if (!m_hasLoadedOk) {
      showErrorPage(tr("Failed to load %1").arg(m_resourceUrl.toString()));
    }
  }
}

void WebViewContainer::onRenderProcessTerminated(
    QWebEnginePage::RenderProcessTerminationStatus status, int exitCode) {
  if (status == QWebEnginePage::NormalTerminationStatus) {
    return;
  }

  qCritical() << "Render process terminated, status:" << status
              << "exit code:" << exitCode;

  m_hasLoadedOk = false;
  ++m_crashCount;
  if (m_crashCount <= MAX_CRASH_RELOADS) {
    // Exponential-ish backoff: 1s, 2s, 3s.
    QTimer::singleShot(1000 * m_crashCount, this, &QWebEngineView::reload);
  } else {
    showErrorPage(tr("The page crashed repeatedly."));
  }
}

void WebViewContainer::showErrorPage(const QString &message) {
  const QString retryHref =
      m_resourceUrl.isValid() ? m_resourceUrl.toString().toHtmlEscaped()
                              : QString();
  const QString retryLink =
      retryHref.isEmpty()
          ? QString()
          : QString("<p><a style='color:#4da3ff' href=\"%1\">Retry</a></p>")
                .arg(retryHref);

  const QString html =
      QString("<html><body style='margin:0;font-family:sans-serif;"
              "background:%1;color:%2;display:flex;align-items:center;"
              "justify-content:center;height:100vh'>"
              "<div style='text-align:center'><h2>%3</h2>%4</div>"
              "</body></html>")
          .arg(m_darkThemeEnabled ? "#2b2b2b" : "#f5f5f5",
               m_darkThemeEnabled ? "#dddddd" : "#333333",
               message.toHtmlEscaped(), retryLink);
  setHtml(html);
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
}

bool WebViewContainer::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::Wheel) {
    QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
    if (wheelEvent->modifiers() & Qt::ControlModifier) {
      const double step = 0.1; // 10% per scroll step
      double currentZoom = zoomFactor();

      if (wheelEvent->angleDelta().y() > 0) {
        currentZoom = qMin(currentZoom + step, 3.0);
      } else {
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
