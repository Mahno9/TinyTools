#include "MainWindow.h"
#include "../core/ClipboardManager.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "../models/WebResource.h"
#include "SettingsDialog.h"
#include "WebViewContainer.h"
#include <QAction>
#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QStackedWidget>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

MainWindow::MainWindow(ClipboardManager *clipboardManager, QWidget *parent)
    : QMainWindow(parent), m_clipboardManager(clipboardManager),
      m_dragging(false), m_resizing(false), m_resizeEdge(Qt::Edges()) {
  qDebug() << "MainWindow::MainWindow() - ENTRY";

  setupUI();
  setupWindowFlags();
  setupWebView();

  // Load window configuration — config already loaded by Application::initialize()
  AppConfig *config = AppConfig::instance();
  resize(config->getWindowWidth(), config->getWindowHeight());
  move(config->getWindowX(), config->getWindowY());
  setWindowOpacity(config->getWindowOpacity() / 100.0);
  if (config->getAlwaysOnTop()) {
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
  }

  // Connect to ResourceManager signals
  connect(ResourceManager::instance(), &ResourceManager::resourcesChanged,
          this, &MainWindow::refreshResources);
  connect(ResourceManager::instance(),
          &ResourceManager::activeResourceChanged, this,
          &MainWindow::switchToResourceById);

  // Initialize resources
  refreshResources();

  // Set initial resource based on startup settings
  WebResource startupResource =
      ResourceManager::instance()->getStartupResource();
  if (startupResource.isValid()) {
    switchToResourceById(startupResource.id);
  } else if (!m_tabResourceIds.isEmpty()) {
    switchToResource(0);
  }
}

void MainWindow::onTabContextMenuRequested(const QPoint &pos) {
  int index = m_tabBar->tabAt(pos);
  if (index != -1) {
    QMenu menu(this);
    QAction *refreshAction = menu.addAction("Refresh");
    connect(refreshAction, &QAction::triggered, this, [this, index]() {
      if (index >= 0 && index < m_tabResourceIds.size()) {
        reloadResource(m_tabResourceIds[index]);
      }
    });
    menu.exec(m_tabBar->mapToGlobal(pos));
  }
}

void MainWindow::onRefreshTriggered() {
  if (!m_currentResourceId.isEmpty()) {
    reloadResource(m_currentResourceId);
  }
}

void MainWindow::reloadResource(const QString &resourceId) {
  QString id = resourceId;
  if (id.isEmpty()) {
    id = m_currentResourceId;
  }

  if (id.isEmpty())
    return;

  if (m_resourceViews.contains(id)) {
    qDebug() << "Reloading resource:" << id;
    m_resourceViews[id]->reloadPage();
  } else {
    // If not loaded yet and we are reloading the CURRENT one, load it.
    // If it's a background tab, we might skip loading it until switched to,
    // but the user explicitly asked for a refresh, so maybe force load?
    // For now, if it's the current one, loadCurrentResource() will do.
    if (id == m_currentResourceId) {
      loadCurrentResource();
    }
  }
}

MainWindow::~MainWindow() {
  // Save window state
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
  m_dragHandle->setStyleSheet("background-color: #2b2b2b;");

  QHBoxLayout *dragHandleLayout = new QHBoxLayout(m_dragHandle);
  dragHandleLayout->setContentsMargins(0, 0, 5, 0);
  dragHandleLayout->setSpacing(5);

  // Settings Button
  m_settingsButton = new QPushButton("⚙", m_dragHandle);
  m_settingsButton->setFixedSize(30, 30);
  m_settingsButton->setStyleSheet(
      "QPushButton { background-color: transparent; color: #fff; border: none; "
      "font-size: 16px; }"
      "QPushButton:hover { background-color: #3b3b3b; }");
  connect(m_settingsButton, &QPushButton::clicked, this,
          &MainWindow::onSettingsRequested);
  dragHandleLayout->addWidget(m_settingsButton);

  // Tab Bar for Resources
  m_tabBar = new QTabBar(m_dragHandle);
  m_tabBar->setDrawBase(false);
  m_tabBar->setStyleSheet(
      "QTabBar::tab { "
      "   background: transparent; color: #aaa; padding: 5px 10px; border: "
      "none; "
      "   min-width: 80px; max-width: 150px; "
      "} "
      "QTabBar::tab:selected { "
      "   color: #fff; background: #3b3b3b; border-bottom: 2px solid #0078d7; "
      "} "
      "QTabBar::tab:hover { "
      "   background: #333; color: #fff; "
      "}");
  connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::onTabChanged);

  // Enable context menu for tabs
  m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tabBar, &QTabBar::customContextMenuRequested, this,
          &MainWindow::onTabContextMenuRequested);

  dragHandleLayout->addWidget(m_tabBar);

  // Initialize Refresh Action (Global Shortcut)
  m_refreshAction = new QAction(this);
  m_refreshAction->setText("Refresh");
  m_refreshAction->setShortcut(QKeySequence("F5"));
  m_refreshAction->setShortcuts({QKeySequence("F5"), QKeySequence("Ctrl+R")});
  connect(m_refreshAction, &QAction::triggered, this,
          &MainWindow::onRefreshTriggered);
  addAction(m_refreshAction); // Add to window to enable shortcuts

  dragHandleLayout->addStretch();

  // Window Controls
  m_minimizeButton = new QPushButton("_", m_dragHandle);
  m_minimizeButton->setFixedSize(30, 30);
  m_minimizeButton->setStyleSheet(
      "QPushButton { background-color: transparent; color: #fff; border: none; "
      "font-weight: bold; }"
      "QPushButton:hover { background-color: #3b3b3b; }");
  connect(m_minimizeButton, &QPushButton::clicked, this,
          &MainWindow::onMinimizeButtonClicked);
  dragHandleLayout->addWidget(m_minimizeButton);

  m_closeButton = new QPushButton("✕", m_dragHandle);
  m_closeButton->setFixedSize(30, 30);
  m_closeButton->setStyleSheet(
      "QPushButton { background-color: transparent; color: #fff; border: none; "
      "}"
      "QPushButton:hover { background-color: #e81123; }");
  connect(m_closeButton, &QPushButton::clicked, this,
          &MainWindow::onCloseButtonClicked);
  dragHandleLayout->addWidget(m_closeButton);

  layout->addWidget(m_dragHandle);
}

void MainWindow::setupWindowFlags() {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_NoSystemBackground);
}

void MainWindow::setupWebView() {
  m_stackedWidget = new QStackedWidget(this);
  centralWidget()->layout()->addWidget(m_stackedWidget);

  // m_webView will track the currently active view
  m_webView = nullptr;

  if (m_clipboardManager) {
    connect(m_clipboardManager, &ClipboardManager::clipboardChanged, this,
            &MainWindow::onClipboardChanged);
  }

  // Debounce timer: zoom saves fire only after 500ms of scroll idle
  m_zoomSaveTimer = new QTimer(this);
  m_zoomSaveTimer->setSingleShot(true);
  m_zoomSaveTimer->setInterval(500);

  QTimer::singleShot(500, this, &MainWindow::applyStartupTheme);
}

void MainWindow::refreshResources() {
  qDebug() << "MainWindow::refreshResources() - ENTRY";

  if (!m_tabBar) return;  // Guard: setupUI() may not have run yet

  // Save current selection if possible
  QString previousId = m_currentResourceId;

  // Block signals to prevent tab change events during rebuild
  m_tabBar->blockSignals(true);

  // Store old count to check if we need to remove tabs
  while (m_tabBar->count() > 0) {
    m_tabBar->removeTab(0);
  }
  m_tabResourceIds.clear();

  QList<WebResource> resources = ResourceManager::instance()->getAllResources();
  qDebug() << "Refreshing resources, total count:" << resources.size();

  for (const WebResource &resource : resources) {
    if (resource.isEnabled) {
      m_tabBar->addTab(resource.name);
      m_tabResourceIds.append(resource.id);
      qDebug() << "Added tab for:" << resource.name;
    } else {
      qDebug() << "Skipped disabled resource:" << resource.name;
    }
  }

  m_tabBar->blockSignals(false);

  // Restore selection or select first
  qDebug() << "Previous ID:" << previousId
           << "New tab count:" << m_tabBar->count();

  if (!previousId.isEmpty()) {
    switchToResourceById(previousId);
    qDebug() << "Restored previous selection";
  } else if (m_tabBar->count() > 0) {
    qDebug() << "Switching to first resource (default)...";
    switchToResource(0);
  } else {
    qWarning() << "No enabled resources available to switch to!";
  }
  qDebug() << "MainWindow::refreshResources() - EXIT";
}

void MainWindow::onTabChanged(int index) {
  if (index >= 0 && index < m_tabResourceIds.size()) {
    QString resourceId = m_tabResourceIds[index];
    qDebug() << "Tab changed to index:" << index
             << "Resource ID:" << resourceId;

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
      // If already on this index (e.g. startup auto-select while signals
      // blocked), manually trigger the change logic
      qDebug() << "Already on tab" << index << "- forcing onTabChanged";
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
  qDebug() << "MainWindow::loadCurrentResource() - ENTRY";
  if (m_currentResourceId.isEmpty()) {
    qWarning() << "m_currentResourceId is empty, cannot load resource";
    return;
  }

  // Check if view already exists
  if (m_resourceViews.contains(m_currentResourceId)) {
    qDebug() << "Switching to existing view for ID:" << m_currentResourceId;
    WebViewContainer *view = m_resourceViews[m_currentResourceId];
    m_stackedWidget->setCurrentWidget(view);
    m_webView = view;

    // Ensure event filter is installed on focusProxy (may not exist at creation
    // time)
    if (view->focusProxy()) {
      view->focusProxy()->installEventFilter(this);
    }
  } else {
    // Create new view
    qDebug() << "Creating new view for ID:" << m_currentResourceId;
    WebResource resource =
        ResourceManager::instance()->getResourceById(m_currentResourceId);

    if (resource.isValid()) {
      WebViewContainer *view = new WebViewContainer(this);

      // Install event filter on both the view and its focus proxy
      // The focus proxy is the actual widget that receives keyboard events
      view->installEventFilter(this);
      if (view->focusProxy()) {
        view->focusProxy()->installEventFilter(this);
      }

      // Connect zoom signal
      connect(view, &WebViewContainer::zoomChanged, this,
              &MainWindow::onZoomChanged);

      // Apply current theme immediately
      bool darkTheme = AppConfig::instance()->getDarkTheme();
      view->applyWebViewTheme(darkTheme);

      view->loadResource(resource);

      m_stackedWidget->addWidget(view);
      m_stackedWidget->setCurrentWidget(view);
      m_resourceViews.insert(m_currentResourceId, view);
      m_webView = view;
    } else {
      qWarning() << "Resource invalid, cannot create view";
    }
  }

  qDebug() << "MainWindow::loadCurrentResource() - EXIT";
}

// ... Existing implementations for other methods ...
// I will copy them back to ensure file completeness, avoiding "empty
// implementation" errors For brevity in this tool call, I'm providing the
// rewritten structure. I will need to be careful not to delete existing logic I
// want to keep.

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

void MainWindow::setOnlineStatus(bool online) {
  if (!m_webView) return;
  if (online) {
    m_webView->reloadPage();
  } else {
    m_webView->setHtml("<html><body><h2>Offline</h2></body></html>");
  }
}

void MainWindow::toggleAlwaysOnTop() {
  Qt::WindowFlags flags = windowFlags();
  if (flags & Qt::WindowStaysOnTopHint) {
    flags &= ~Qt::WindowStaysOnTopHint;
  } else {
    flags |= Qt::WindowStaysOnTopHint;
  }
  setWindowFlags(flags);
  show();
}

void MainWindow::applyWebViewTheme(bool darkTheme) {
  // Apply to all loaded views
  for (auto view : m_resourceViews) {
    view->applyWebViewTheme(darkTheme);
  }

  // Apply theme to drag handle and header elements
  if (darkTheme) {
    // Dark theme colors
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
    // Light theme colors
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
  // Create dialog WITHOUT parent to avoid modality issues with frameless window
  // The parent relationship was causing nativeEvent to stop working on Windows
  SettingsDialog dialog(nullptr);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setAttribute(Qt::WA_DeleteOnClose, false);
  // Ensure dialog appears above the main window (which may have
  // WindowStaysOnTopHint)
  dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowStaysOnTopHint);

  // Store original opacity to revert if cancelled
  int originalOpacity = AppConfig::instance()->getWindowOpacity();

  // Connect immediate update signal for "Apply" button or live changes
  connect(&dialog, &SettingsDialog::testOpacity, this, &MainWindow::setOpacity);

  if (dialog.exec() == QDialog::Accepted) {
    // Apply settings after dialog closes (already saved by dialog)
    applySettings();
  } else {
    // Revert opacity if cancelled (and other visual settings if we live-updated
    // them)
    setOpacity(originalOpacity);
  }

  // Ensure the main window is properly reactivated
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

  // Apply theme changes
  applyWebViewTheme(config->getDarkTheme());

  // Refresh resources in case they changed
  refreshResources();
}

void MainWindow::applyStartupTheme() {
  bool dark = AppConfig::instance()->getDarkTheme();
  applyWebViewTheme(dark);
}

void MainWindow::setOpacity(int value) {
  // Use Qt's method as baseline
  setWindowOpacity(value / 100.0);

#ifdef Q_OS_WIN
  // Direct Windows API approach: SetLayeredWindowAttributes
  // This bypasses any Qt caching and directly tells Windows to update the
  // alpha.
  HWND hwnd = (HWND)winId();
  if (hwnd) {
    // Ensure WS_EX_LAYERED style is set (should be, for frameless translucent
    // windows)
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (!(exStyle & WS_EX_LAYERED)) {
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }

    // Set alpha directly: value is 0-100, Windows expects 0-255
    BYTE alpha = static_cast<BYTE>((value * 255) / 100);
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
  }
#endif
}

void MainWindow::onClipboardChanged(const QString &text) {
  if (AppConfig::instance()->getAutoTranslate() && !text.isEmpty()) {
    showAndActivate();
    if (m_webView)
      m_webView->insertText(text);
  }
}

void MainWindow::onCloseButtonClicked() { hide(); }

void MainWindow::onMinimizeButtonClicked() { showMinimized(); }

void MainWindow::onSettingsButtonClicked() { onSettingsRequested(); }

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
  // Intercept key events from WebView to handle Alt+Number tab switching
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->modifiers() & Qt::AltModifier) {
      int key = keyEvent->key();
      if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        int index = key - Qt::Key_1;
        switchToResource(index);
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
      int index = key - Qt::Key_1;
      switchToResource(index);
      event->accept();
      return;
    }
  }
  QMainWindow::keyPressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  hide();
  event->ignore();
}

void MainWindow::changeEvent(QEvent *event) {
  QMainWindow::changeEvent(event);
  if (event->type() == QEvent::WindowStateChange) {
    if (isMinimized())
      hide();
  }
}

void MainWindow::hideEvent(QHideEvent *event) {
  // Save window geometry when hiding
  if (!isMinimized() && !isMaximized()) {
    AppConfig *config = AppConfig::instance();
    config->setWindowWidth(width());
    config->setWindowHeight(height());
    config->setWindowX(x());
    config->setWindowY(y());
    config->save();
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

    // Use lParam screen coordinates — more accurate than QCursor::pos() under
    // fast movement and multi-monitor DPI scaling.
    QPoint localPos = mapFromGlobal(
        QPoint(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)));

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
  qDebug() << "MainWindow::onZoomChanged -" << resourceId << zoomFactor;

  // Update in-memory state immediately
  WebResource resource =
      ResourceManager::instance()->getResourceById(resourceId);
  if (resource.isValid()) {
    resource.zoomFactor = zoomFactor;
    ResourceManager::instance()->updateResource(resource);
  }

  // Debounce: restart timer on every scroll tick; only persist after 500ms idle
  // This prevents blocking disk I/O on every Ctrl+wheel event.
  if (m_zoomSaveTimer) {
    m_zoomSaveTimer->disconnect();
    connect(m_zoomSaveTimer, &QTimer::timeout, this, []() {
      ResourceManager::instance()->saveToConfig();
    });
    m_zoomSaveTimer->start();
  }
}
