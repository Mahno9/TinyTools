#include "MainWindow.h"
#include "../core/ClipboardManager.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "../models/WebResource.h"
#include "SettingsDialog.h"
#include "WebViewContainer.h"
#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
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

  try {
    setupUI();
    setupWindowFlags();
    setupWebView();

    // Load window configuration
    AppConfig *config = AppConfig::instance();
    if (config->load()) {
      resize(config->getWindowWidth(), config->getWindowHeight());
      move(config->getWindowX(), config->getWindowY());
      setWindowOpacity(config->getWindowOpacity() / 100.0);
      if (config->getAlwaysOnTop()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
      }
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

  } catch (const std::exception &e) {
    qCritical() << "MainWindow construction failed:" << e.what();
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
  dragHandleLayout->addWidget(m_tabBar);

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

  QTimer::singleShot(500, this, &MainWindow::applyStartupTheme);
}

void MainWindow::refreshResources() {
  qDebug() << "MainWindow::refreshResources() - ENTRY";

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
  } else {
    // Create new view
    qDebug() << "Creating new view for ID:" << m_currentResourceId;
    WebResource resource =
        ResourceManager::instance()->getResourceById(m_currentResourceId);

    if (resource.isValid()) {
      WebViewContainer *view = new WebViewContainer(this);

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
  show();
  raise();
  activateWindow();

  // Ensure on screen
  QScreen *screen = QGuiApplication::screenAt(pos());
  if (!screen)
    move(100, 100);
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
  if (m_webView) {
    if (online) {
      // m_webView->reload();
    } else {
      m_webView->setHtml("<html><body><h2>Offline</h2></body></html>");
    }
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

  dialog.exec();

  // Apply settings after dialog closes
  applySettings();

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

void MainWindow::setOpacity(int value) { setWindowOpacity(value / 100.0); }

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

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message,
                             qintptr *result) {
  MSG *msg = static_cast<MSG *>(message);
  if (msg->message == WM_NCHITTEST) {
    if (isMaximized())
      return false;

    QPoint localPos = mapFromGlobal(QCursor::pos());

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

  // Update resource in ResourceManager
  WebResource resource =
      ResourceManager::instance()->getResourceById(resourceId);
  if (resource.isValid()) {
    resource.zoomFactor = zoomFactor;
    ResourceManager::instance()->updateResource(resource);

    // Save immediately
    ResourceManager::instance()->saveToConfig();
  }
}
