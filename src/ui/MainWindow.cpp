#include "MainWindow.h"
#include "../core/ClipboardManager.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "../models/WebResource.h"
#include "SettingsDialog.h"
#include "WebViewContainer.h"
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QPushButton>
#include <QScreen>
#include <QStackedWidget>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

MainWindow::MainWindow(ClipboardManager *clipboardManager, QWidget *parent)
    : QMainWindow(parent), m_clipboardManager(clipboardManager) {
  AppConfig *config = AppConfig::instance();

  setupUI();
  setupWindowFlags(config->getAlwaysOnTop());
  setupWebView();

  resize(config->getWindowWidth(), config->getWindowHeight());
  move(config->getWindowX(), config->getWindowY());
  setWindowOpacity(config->getWindowOpacity() / 100.0);
  applyWebViewTheme(config->getDarkTheme());

  // Connect to ResourceManager signals
  ResourceManager *rm = ResourceManager::instance();
  connect(rm, &ResourceManager::resourcesChanged, this,
          &MainWindow::refreshResources);
  connect(rm, &ResourceManager::activeResourceChanged, this,
          &MainWindow::switchToResourceById);
  connect(rm, &ResourceManager::resourceRemoved, this,
          &MainWindow::onResourceRemoved);
  connect(rm, &ResourceManager::resourceUpdated, this,
          &MainWindow::onResourceUpdated);

  // Initialize resources
  refreshResources();

  // Set initial resource based on startup settings
  WebResource startupResource = rm->getStartupResource();
  if (startupResource.isValid()) {
    switchToResourceById(startupResource.id);
  } else if (!m_tabResourceIds.isEmpty()) {
    switchToResource(0);
  }
}

MainWindow::~MainWindow() { saveGeometryToConfig(); }

void MainWindow::saveGeometryToConfig() {
  AppConfig *config = AppConfig::instance();
  config->setWindowWidth(width());
  config->setWindowHeight(height());
  config->setWindowX(x());
  config->setWindowY(y());
  config->save();
}

void MainWindow::setupUI() {
  QWidget *centralWidget = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(centralWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  setCentralWidget(centralWidget);

  // Custom Title Bar (Drag Handle)
  m_dragHandle = new QWidget(this);
  m_dragHandle->setFixedHeight(30);

  QHBoxLayout *dragHandleLayout = new QHBoxLayout(m_dragHandle);
  dragHandleLayout->setContentsMargins(0, 0, 5, 0);
  dragHandleLayout->setSpacing(5);

  // Settings Button
  m_settingsButton = new QPushButton("⚙", m_dragHandle);
  m_settingsButton->setFixedSize(30, 30);
  connect(m_settingsButton, &QPushButton::clicked, this,
          &MainWindow::onSettingsRequested);
  dragHandleLayout->addWidget(m_settingsButton);

  // Tab Bar for Resources
  m_tabBar = new QTabBar(m_dragHandle);
  m_tabBar->setDrawBase(false);
  connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::onTabChanged);

  // Enable context menu for tabs
  m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tabBar, &QTabBar::customContextMenuRequested, this,
          &MainWindow::onTabContextMenuRequested);

  dragHandleLayout->addWidget(m_tabBar);

  // Refresh shortcuts (F5 / Ctrl+R)
  m_refreshAction = new QAction("Refresh", this);
  m_refreshAction->setShortcuts({QKeySequence("F5"), QKeySequence("Ctrl+R")});
  connect(m_refreshAction, &QAction::triggered, this,
          &MainWindow::reloadCurrentResource);
  addAction(m_refreshAction);

  dragHandleLayout->addStretch();

  // Window Controls
  m_minimizeButton = new QPushButton("_", m_dragHandle);
  m_minimizeButton->setFixedSize(30, 30);
  connect(m_minimizeButton, &QPushButton::clicked, this,
          &MainWindow::showMinimized);
  dragHandleLayout->addWidget(m_minimizeButton);

  m_closeButton = new QPushButton("✕", m_dragHandle);
  m_closeButton->setFixedSize(30, 30);
  // close() (not hide()) so closeEvent can honor the Minimize-to-Tray setting.
  connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::close);
  dragHandleLayout->addWidget(m_closeButton);

  layout->addWidget(m_dragHandle);
}

void MainWindow::setupWindowFlags(bool alwaysOnTop) {
  Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint;
  if (alwaysOnTop) {
    flags |= Qt::WindowStaysOnTopHint;
  }
  setWindowFlags(flags);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_NoSystemBackground);
}

void MainWindow::setupWebView() {
  m_stackedWidget = new QStackedWidget(this);
  centralWidget()->layout()->addWidget(m_stackedWidget);

  // m_webView tracks the currently active view
  m_webView = nullptr;

  // Debounce timer: zoom saves fire only after 500ms of scroll idle
  m_zoomSaveTimer = new QTimer(this);
  m_zoomSaveTimer->setSingleShot(true);
  m_zoomSaveTimer->setInterval(500);
  connect(m_zoomSaveTimer, &QTimer::timeout, this,
          []() { ResourceManager::instance()->saveToConfig(); });
}

void MainWindow::refreshResources() {
  if (!m_tabBar)
    return; // Guard: setupUI() may not have run yet

  // Save current selection if possible
  QString previousId = m_currentResourceId;

  // Block signals to prevent tab change events during rebuild
  m_tabBar->blockSignals(true);

  while (m_tabBar->count() > 0) {
    m_tabBar->removeTab(0);
  }
  m_tabResourceIds.clear();

  const QList<WebResource> resources =
      ResourceManager::instance()->getAllResources();
  for (const WebResource &resource : resources) {
    if (resource.isEnabled) {
      m_tabBar->addTab(resource.name);
      m_tabResourceIds.append(resource.id);
    }
  }

  m_tabBar->blockSignals(false);

  // Restore selection or select first
  if (!previousId.isEmpty() && m_tabResourceIds.contains(previousId)) {
    switchToResourceById(previousId);
  } else if (m_tabBar->count() > 0) {
    switchToResource(0);
  } else {
    qWarning() << "No enabled resources available";
  }
}

void MainWindow::onTabChanged(int index) {
  if (index >= 0 && index < m_tabResourceIds.size()) {
    QString resourceId = m_tabResourceIds[index];
    if (resourceId != m_currentResourceId) {
      m_currentResourceId = resourceId;
      ResourceManager::instance()->setLastUsedResourceId(resourceId);
      loadCurrentResource();
    }
  }
}

void MainWindow::switchToResource(int index) {
  if (index >= 0 && index < m_tabBar->count()) {
    if (m_tabBar->currentIndex() == index) {
      // Already on this index (e.g. startup auto-select while signals
      // blocked): trigger the change logic manually.
      onTabChanged(index);
    } else {
      m_tabBar->setCurrentIndex(index);
    }
  }
}

void MainWindow::switchToResourceById(const QString &id) {
  int index = m_tabResourceIds.indexOf(id);
  if (index != -1) {
    switchToResource(index);
  }
}

void MainWindow::loadCurrentResource() {
  if (m_currentResourceId.isEmpty()) {
    return;
  }

  if (m_resourceViews.contains(m_currentResourceId)) {
    WebViewContainer *view = m_resourceViews[m_currentResourceId];
    m_stackedWidget->setCurrentWidget(view);
    m_webView = view;

    // Ensure event filter is installed on focusProxy (may not exist at
    // creation time)
    if (view->focusProxy()) {
      view->focusProxy()->installEventFilter(this);
    }
    return;
  }

  // Create new view
  WebResource resource =
      ResourceManager::instance()->getResourceById(m_currentResourceId);
  if (!resource.isValid()) {
    qWarning() << "Resource invalid, cannot create view:" << m_currentResourceId;
    return;
  }

  WebViewContainer *view = new WebViewContainer(this);

  // Install event filter on both the view and its focus proxy.
  // The focus proxy is the actual widget that receives keyboard events.
  view->installEventFilter(this);
  if (view->focusProxy()) {
    view->focusProxy()->installEventFilter(this);
  }

  connect(view, &WebViewContainer::zoomChanged, this,
          &MainWindow::onZoomChanged);

  view->applyWebViewTheme(AppConfig::instance()->getDarkTheme());
  view->loadResource(resource);

  m_stackedWidget->addWidget(view);
  m_stackedWidget->setCurrentWidget(view);
  m_resourceViews.insert(m_currentResourceId, view);
  m_webView = view;
}

void MainWindow::onResourceRemoved(const QString &resourceId) {
  WebViewContainer *view = m_resourceViews.take(resourceId);
  if (!view)
    return;

  m_stackedWidget->removeWidget(view);
  if (m_webView == view) {
    m_webView = nullptr;
  }
  if (m_currentResourceId == resourceId) {
    m_currentResourceId.clear();
  }
  // Frees the underlying Chromium renderer instead of leaking it until exit.
  view->deleteLater();
}

void MainWindow::onResourceUpdated(const QString &resourceId) {
  WebViewContainer *view = m_resourceViews.value(resourceId);
  if (!view)
    return;

  WebResource resource =
      ResourceManager::instance()->getResourceById(resourceId);
  if (resource.isValid() && view->resourceUrl() != QUrl(resource.url)) {
    view->loadResource(resource); // URL changed: reload the tab now
  }
}

void MainWindow::reloadCurrentResource() {
  if (m_currentResourceId.isEmpty())
    return;
  if (m_resourceViews.contains(m_currentResourceId)) {
    m_resourceViews[m_currentResourceId]->reloadPage();
  } else {
    loadCurrentResource();
  }
}

void MainWindow::onTabContextMenuRequested(const QPoint &pos) {
  int index = m_tabBar->tabAt(pos);
  if (index == -1)
    return;

  QMenu menu(this);
  QAction *refreshAction = menu.addAction("Refresh");
  connect(refreshAction, &QAction::triggered, this, [this, index]() {
    if (index >= 0 && index < m_tabResourceIds.size()) {
      const QString id = m_tabResourceIds[index];
      if (m_resourceViews.contains(id)) {
        m_resourceViews[id]->reloadPage();
      } else if (id == m_currentResourceId) {
        loadCurrentResource();
      }
    }
  });
  menu.exec(m_tabBar->mapToGlobal(pos));
}

void MainWindow::showAndActivate() {
  if (isMinimized()) {
    showNormal();
  }
  show();

#ifdef Q_OS_WIN
  HWND hwnd = (HWND)winId();
  if (hwnd) {
    DWORD currentThreadId = GetCurrentThreadId();
    HWND foreground = GetForegroundWindow();
    if (foreground) {
      // Guard: GetForegroundWindow() can return NULL on secure desktop
      DWORD foregroundThreadId = GetWindowThreadProcessId(foreground, NULL);
      if (foregroundThreadId && currentThreadId != foregroundThreadId) {
        AttachThreadInput(foregroundThreadId, currentThreadId, TRUE);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        AttachThreadInput(foregroundThreadId, currentThreadId, FALSE);
      } else {
        SetForegroundWindow(hwnd);
      }
    } else {
      SetForegroundWindow(hwnd);
    }
  }
#endif

  raise();
  activateWindow();

  // Ensure on screen
  QScreen *screen = QGuiApplication::screenAt(pos());
  if (!screen)
    move(100, 100);

  if (m_webView) {
    m_webView->setFocus();
  }
}

void MainWindow::insertClipboardText(bool useAltScript) {
  if (!m_clipboardManager || !m_webView)
    return;

  QString text = m_clipboardManager->getText();
  if (!text.isEmpty()) {
    if (useAltScript) {
      m_webView->insertAltText(text);
    } else {
      m_webView->insertText(text);
    }
  }
}

void MainWindow::toggleAlwaysOnTop() {
  const bool wasVisible = isVisible();
  Qt::WindowFlags flags = windowFlags();
  if (flags & Qt::WindowStaysOnTopHint) {
    flags &= ~Qt::WindowStaysOnTopHint;
  } else {
    flags |= Qt::WindowStaysOnTopHint;
  }
  // setWindowFlags() hides the window; only re-show if it was visible so
  // toggling from the tray does not pop up a hidden window.
  setWindowFlags(flags);
  if (wasVisible) {
    show();
  }
}

void MainWindow::applyWebViewTheme(bool darkTheme) {
  // Apply to all loaded views
  for (auto view : m_resourceViews) {
    view->applyWebViewTheme(darkTheme);
  }

  // Apply theme to drag handle and header elements
  if (darkTheme) {
    m_dragHandle->setStyleSheet("background-color: #2b2b2b;");
    m_settingsButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #fff; border: "
        "none; font-size: 16px; }"
        "QPushButton:hover { background-color: #3b3b3b; }");
    m_minimizeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #fff; border: "
        "none; font-weight: bold; }"
        "QPushButton:hover { background-color: #3b3b3b; }");
    m_closeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #fff; border: "
        "none; }"
        "QPushButton:hover { background-color: #e81123; }");
    m_tabBar->setStyleSheet("QTabBar::tab { "
                            "   background: transparent; color: #aaa; padding: "
                            "5px 10px; border: none; "
                            "   min-width: 80px; max-width: 150px; "
                            "} "
                            "QTabBar::tab:selected { "
                            "   color: #fff; background: #3b3b3b; "
                            "border-bottom: 2px solid #0078d7; "
                            "} "
                            "QTabBar::tab:hover { "
                            "   background: #333; color: #fff; "
                            "}");
  } else {
    m_dragHandle->setStyleSheet("background-color: #f0f0f0;");
    m_settingsButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #333; border: "
        "none; font-size: 16px; }"
        "QPushButton:hover { background-color: #ddd; }");
    m_minimizeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #333; border: "
        "none; font-weight: bold; }"
        "QPushButton:hover { background-color: #ddd; }");
    m_closeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #333; border: "
        "none; }"
        "QPushButton:hover { background-color: #e81123; color: #fff; }");
    m_tabBar->setStyleSheet("QTabBar::tab { "
                            "   background: transparent; color: #555; padding: "
                            "5px 10px; border: none; "
                            "   min-width: 80px; max-width: 150px; "
                            "} "
                            "QTabBar::tab:selected { "
                            "   color: #000; background: #e0e0e0; "
                            "border-bottom: 2px solid #0078d7; "
                            "} "
                            "QTabBar::tab:hover { "
                            "   background: #ddd; color: #000; "
                            "}");
  }
}

void MainWindow::onSettingsRequested() {
  // Create dialog WITHOUT parent to avoid modality issues with frameless
  // window. The parent relationship was causing nativeEvent to stop working
  // on Windows.
  SettingsDialog dialog(nullptr);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setAttribute(Qt::WA_DeleteOnClose, false);
  // Ensure dialog appears above the main window (which may have
  // WindowStaysOnTopHint)
  dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowStaysOnTopHint);

  // Store original opacity to revert if cancelled
  int originalOpacity = AppConfig::instance()->getWindowOpacity();

  // Live opacity preview while the user drags the spinbox
  connect(&dialog, &SettingsDialog::testOpacity, this, &MainWindow::setOpacity);

  if (dialog.exec() == QDialog::Accepted) {
    applySettings();
  } else {
    // Revert live-previewed opacity
    setOpacity(originalOpacity);
  }

  activateWindow();
  raise();
}

void MainWindow::applySettings() {
  AppConfig *config = AppConfig::instance();
  setWindowOpacity(config->getWindowOpacity() / 100.0);

  bool alwaysOnTop = config->getAlwaysOnTop();
  bool currentlyOnTop = (windowFlags() & Qt::WindowStaysOnTopHint);
  if (alwaysOnTop != currentlyOnTop) {
    toggleAlwaysOnTop();
  }

  applyWebViewTheme(config->getDarkTheme());
}

void MainWindow::setOpacity(int value) {
  setWindowOpacity(value / 100.0);

#ifdef Q_OS_WIN
  // Direct Windows API: SetLayeredWindowAttributes bypasses Qt caching and
  // updates the alpha immediately (needed for live preview).
  HWND hwnd = (HWND)winId();
  if (hwnd) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (!(exStyle & WS_EX_LAYERED)) {
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }
    BYTE alpha = static_cast<BYTE>((value * 255) / 100);
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
  }
#endif
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
  // Intercept key events from WebView to handle Alt+Number tab switching
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->modifiers() & Qt::AltModifier) {
      int key = keyEvent->key();
      if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        switchToResource(key - Qt::Key_1);
        return true; // Event handled, don't propagate to WebView
      }
    }
  }
  return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  // Handle Alt + Number for tab switching
  if (event->modifiers() & Qt::AltModifier) {
    int key = event->key();
    if (key >= Qt::Key_1 && key <= Qt::Key_9) {
      switchToResource(key - Qt::Key_1);
      event->accept();
      return;
    }
  }
  QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (AppConfig::instance()->getMinimizeToTray()) {
    hide();
    event->ignore();
  } else {
    saveGeometryToConfig();
    event->accept();
    QApplication::quit();
  }
}

void MainWindow::changeEvent(QEvent *event) {
  QMainWindow::changeEvent(event);
  if (event->type() == QEvent::WindowStateChange) {
    if (isMinimized() && AppConfig::instance()->getMinimizeToTray()) {
      hide();
    }
  }
}

void MainWindow::hideEvent(QHideEvent *event) {
  // Save window geometry when hiding
  if (!isMinimized() && !isMaximized()) {
    saveGeometryToConfig();
  }
  QMainWindow::hideEvent(event);
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message,
                             qintptr *result) {
  MSG *msg = static_cast<MSG *>(message);
  if (msg->message == WM_NCHITTEST) {
    if (isMaximized())
      return false;

    // Use ScreenToClient for physical→client conversion, then scale to logical.
    // lParam gives physical screen coords; mapFromGlobal expects logical coords,
    // so naive QPoint(GET_X_LPARAM, GET_Y_LPARAM) breaks on DPI != 100%.
    POINT pt = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
    ScreenToClient(reinterpret_cast<HWND>(winId()), &pt);
    qreal dpr = devicePixelRatioF();
    QPoint localPos(qRound(pt.x / dpr), qRound(pt.y / dpr));

    int w = width();
    int h = height();
    int margin = RESIZE_MARGIN;

    bool left = localPos.x() < margin;
    bool right = localPos.x() >= w - margin;
    bool top = localPos.y() < margin;
    bool bottom = localPos.y() >= h - margin;

    if (top && left) {
      *result = HTTOPLEFT;
      return true;
    }
    if (top && right) {
      *result = HTTOPRIGHT;
      return true;
    }
    if (bottom && left) {
      *result = HTBOTTOMLEFT;
      return true;
    }
    if (bottom && right) {
      *result = HTBOTTOMRIGHT;
      return true;
    }
    if (left) {
      *result = HTLEFT;
      return true;
    }
    if (right) {
      *result = HTRIGHT;
      return true;
    }
    if (top) {
      *result = HTTOP;
      return true;
    }
    if (bottom) {
      *result = HTBOTTOM;
      return true;
    }

    // Title bar (HTCAPTION)
    if (localPos.y() <= 30) {
      QWidget *child = childAt(localPos);
      if (qobject_cast<QPushButton *>(child) ||
          qobject_cast<QTabBar *>(child)) {
        return false;
      }
      *result = HTCAPTION;
      return true;
    }
  }
  return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::onZoomChanged(const QString &resourceId, double zoomFactor) {
  // Silent in-memory update: no resourcesChanged signal, so per-tick zoom
  // does not rebuild the tab bar.
  ResourceManager::instance()->setResourceZoom(resourceId, zoomFactor);

  // Debounce: restart timer on every scroll tick; persist after 500ms idle
  if (m_zoomSaveTimer) {
    m_zoomSaveTimer->start();
  }
}
