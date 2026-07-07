#pragma once
#include <QObject>
#include <QPointer>

#include "../core/HotkeyManager.h"

class MainWindow;
class TrayIcon;
class ClipboardManager;

class Application : public QObject {
  Q_OBJECT

public:
  explicit Application(QObject *parent = nullptr);
  ~Application();

  void initialize();

private slots:
  void onHotkeyPressed(int type);
  void onSettingsChanged();

private:
  void setupComponents();
  void connectSignals();
  QStringList registerAllHotkeys();
  QStringList updateAllHotkeys();
  void notifyHotkeyFailures(const QStringList &failed);

  QPointer<MainWindow> m_mainWindow;
  QPointer<TrayIcon> m_trayIcon;
  QPointer<HotkeyManager> m_hotkeyManager;
  QPointer<ClipboardManager> m_clipboardManager;

  Q_DISABLE_COPY(Application)
};
