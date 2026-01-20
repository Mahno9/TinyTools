#include "WebViewContainer.h"
#include "../models/ResourceManager.h"
#include <QChildEvent>
#include <QContextMenuEvent>
#include <QDebug>
#include <QTimer>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWheelEvent>

WebViewContainer::WebViewContainer(QWidget *parent)
    : QWebEngineView(parent), m_darkThemeEnabled(false),
      m_darkThemeApplied(false) {
  // Configure page
  QWebEnginePage *page = new QWebEnginePage(this);
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

  // Connect signals
  connect(page, &QWebEnginePage::loadFinished, this,
          &WebViewContainer::onLoadFinished);
  connect(page, &QWebEnginePage::loadProgress, this,
          &WebViewContainer::onLoadProgress);
  connect(page, &QWebEnginePage::renderProcessTerminated, this,
          &WebViewContainer::onRenderProcessTerminated);

  // Initial load blank or default?
  // We wait for loadResource to be called.
  // load(QUrl("about:blank"));

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

  // Get fresh script from ResourceManager (in case it was updated in settings)
  WebResource resource =
      ResourceManager::instance()->getResourceById(m_resourceId);
  QString openScript = resource.isValid() ? resource.openScript : m_openScript;

  if (openScript.isEmpty()) {
    qDebug() << "No open script defined for this resource";
    return;
  }

  // Replace %1 with escaped text
  QString safeText = text;
  safeText.replace("\\", "\\\\");
  safeText.replace("'", "\\'");
  safeText.replace("\"", "\\\"");
  safeText.replace("\n", "\\n");
  safeText.replace("\r", "");

  QString script = openScript;
  script.replace("%1", safeText);
  script.replace("%CLIPBOARD%", safeText);

  qDebug() << "Executing open script with text length:" << text.length();
  injectJavaScript(script);
}

void WebViewContainer::insertAltText(const QString &text) {
  if (isLoading()) {
    qWarning() << "Cannot execute alt script: page is loading";
    return;
  }

  // Get fresh script from ResourceManager (in case it was updated in settings)
  WebResource resource =
      ResourceManager::instance()->getResourceById(m_resourceId);
  QString altScript =
      resource.isValid() ? resource.altOpenScript : m_altOpenScript;

  if (altScript.isEmpty()) {
    qDebug() << "No alternative open script defined for this resource";
    return;
  }

  QString safeText = text;
  safeText.replace("\\", "\\\\");
  safeText.replace("'", "\\'");
  safeText.replace("\"", "\\\"");
  safeText.replace("\n", "\\n");
  safeText.replace("\r", "");

  QString script = altScript;
  script.replace("%1", safeText);
  script.replace("%CLIPBOARD%", safeText);

  qDebug() << "Executing alternative open script with text length:"
           << text.length();
  injectJavaScript(script);
}

void WebViewContainer::reloadTranslator() { reload(); }

bool WebViewContainer::isLoading() const {
  return page() && page()->isLoading();
}

void WebViewContainer::onLoadFinished(bool ok) {
  if (ok) {
    qDebug() << "Page loaded successfully";
    emit pageLoaded(true);

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
  // Removed hardcoded Yandex theme logic.
  // TODO: Allow generic theme scripts?
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
