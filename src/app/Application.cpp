#include "Application.h"
#include "../core/ClipboardManager.h"
#include "../core/HotkeyManager.h"
#include "../models/AppConfig.h"
#include "../models/ResourceManager.h"
#include "../models/WebResource.h"
#include "../tray/TrayIcon.h"
#include "../ui/MainWindow.h"
#include <QApplication>
#include <QDebug>

Application::Application(QObject *parent) : QObject(parent) {}

Application::~Application() {
  // Delete MainWindow first: its WebViewContainers/QWebEnginePages must be gone
  // before QApplication (later) destroys the WebEngine profile (its child).
  delete m_mainWindow.data();
  // Explicitly delete singletons while QApplication is still alive.
  ResourceManager::cleanupInstance();
  AppConfig::cleanupInstance();
}

void Application::initialize() {
  if (!AppConfig::instance()->load()) {
    qWarning() << "Failed to load configuration - using defaults";
  }

  ResourceManager::instance()->loadFromConfig();

  setupComponents();
  connectSignals();

  if (AppConfig::instance()->getShowWindowOnStartup()) {
    m_mainWindow->show();
  } else {
    m_mainWindow->hide();
  }

  qInfo() << "Application initialized,"
          << ResourceManager::instance()->getResourceCount() << "resources";
}

void Application::setupComponents() {
  m_clipboardManager = new ClipboardManager(this);
  m_hotkeyManager = new HotkeyManager(this);

  m_mainWindow = new MainWindow(m_clipboardManager, nullptr);

  m_trayIcon = new TrayIcon(m_mainWindow, this);
  m_trayIcon->show();

  // Register hotkeys after the tray icon exists so failures can be reported
  // to the user, not just the log.
  notifyHotkeyFailures(registerAllHotkeys());
}

void Application::connectSignals() {
  connect(m_hotkeyManager, &HotkeyManager::hotkeyPressed, this,
          &Application::onHotkeyPressed);

  connect(AppConfig::instance(), &AppConfig::settingsChanged, this,
          &Application::onSettingsChanged, Qt::UniqueConnection);

  connect(m_trayIcon, &TrayIcon::showWindowRequested, m_mainWindow,
          &MainWindow::showAndActivate);
  connect(m_trayIcon, &TrayIcon::hideWindowRequested, m_mainWindow,
          &MainWindow::hide);
  connect(m_trayIcon, &TrayIcon::quitRequested, this,
          []() { QApplication::quit(); });
}

QStringList Application::registerAllHotkeys() {
  QStringList failed;
  for (int i = 0; i < HotkeyType::Count; ++i) {
    HotkeyType::Type type = static_cast<HotkeyType::Type>(i);

    int key = AppConfig::instance()->getHotkeyKey(type);
    Qt::KeyboardModifiers modifiers =
        AppConfig::instance()->getHotkeyModifiers(type);

    if (!m_hotkeyManager->registerHotkey(type, key, modifiers)) {
      failed << QString("%1 (%2)")
                    .arg(HotkeyType::toDisplayName(type),
                         QKeySequence(key | modifiers).toString());
    }
  }
  return failed;
}

QStringList Application::updateAllHotkeys() {
  QStringList failed;
  for (int i = 0; i < HotkeyType::Count; ++i) {
    HotkeyType::Type type = static_cast<HotkeyType::Type>(i);

    int key = AppConfig::instance()->getHotkeyKey(type);
    Qt::KeyboardModifiers modifiers =
        AppConfig::instance()->getHotkeyModifiers(type);

    if (!m_hotkeyManager->updateHotkey(type, key, modifiers)) {
      failed << QString("%1 (%2)")
                    .arg(HotkeyType::toDisplayName(type),
                         QKeySequence(key | modifiers).toString());
    }
  }
  return failed;
}

void Application::notifyHotkeyFailures(const QStringList &failed) {
  if (failed.isEmpty() || !m_trayIcon) {
    return;
  }
  m_trayIcon->showNotification(
      tr("Hotkey not available"),
      tr("Failed to register: %1. The combination may be taken by another "
         "application - change it in Settings.")
          .arg(failed.join(", ")));
}

void Application::onHotkeyPressed(int type) {
  if (!m_mainWindow) {
    return;
  }

  // Toggle behavior: if window is visible, hide it; otherwise show and run
  // the resource script.
  if (m_mainWindow->isVisible() && !m_mainWindow->isMinimized()) {
    m_mainWindow->hide();
    return;
  }

  m_mainWindow->showAndActivate();

  // Startup behavior: switch to the default tab if configured
  if (ResourceManager::instance()->getStartupMode() ==
      ResourceManager::SelectedResource) {
    m_mainWindow->switchToResourceById(
        ResourceManager::instance()->getDefaultResourceId());
  }

  switch (static_cast<HotkeyType::Type>(type)) {
  case HotkeyType::MainToggle:
    m_mainWindow->insertClipboardText(false /* Use Main Script */);
    break;
  case HotkeyType::AlternativeToggle:
    m_mainWindow->insertClipboardText(true /* Use Alt Script */);
    break;
  default:
    qWarning() << "Unknown hotkey type:" << type;
    break;
  }
}

void Application::onSettingsChanged() {
  if (m_mainWindow) {
    m_mainWindow->applyWebViewTheme(AppConfig::instance()->getDarkTheme());
  }
  if (m_hotkeyManager) {
    // No-op for hotkeys that did not change (updateHotkey checks), so
    // geometry-only saves do not churn global hotkey registration.
    notifyHotkeyFailures(updateAllHotkeys());
  }
}
