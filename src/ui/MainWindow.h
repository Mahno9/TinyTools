#pragma once
#include <QMainWindow>
#include <QMap>
#include <QPointer>

class WebViewContainer;
class ClipboardManager;
class QSystemTrayIcon;
class QPushButton;
class QMouseEvent;
class QCloseEvent;
class QTabBar;
class QStackedWidget;
class QEvent;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(ClipboardManager *clipboardManager,
                      QWidget *parent = nullptr);
  ~MainWindow();

  void showAndActivate();
  void insertClipboardText(bool useAltScript = false);
  void setOnlineStatus(bool online);
  void toggleAlwaysOnTop();
  void applyWebViewTheme(bool darkTheme);
  void switchToResource(int index);
  void switchToResourceById(const QString &id);

public slots:
  void onSettingsRequested();
  void applyStartupTheme();
  void refreshResources();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;
#ifdef Q_OS_WIN
  bool nativeEvent(const QByteArray &eventType, void *message,
                   qintptr *result) override;
#endif

private slots:
  void setOpacity(int value);
  void onClipboardChanged(const QString &text);
  void onCloseButtonClicked();
  void onMinimizeButtonClicked();
  void onSettingsButtonClicked();
  void onTabChanged(int index);
  void onZoomChanged(const QString &resourceId, double zoomFactor);

private:
  void setupUI();
  void setupWindowFlags();
  void setupWebView();
  void applySettings();
  void loadCurrentResource();

  QPointer<ClipboardManager> m_clipboardManager;

  QWidget *m_dragHandle;
  QStackedWidget *m_stackedWidget;
  QMap<QString, WebViewContainer *> m_resourceViews;
  WebViewContainer *m_webView = nullptr;

  QPushButton *m_settingsButton;
  QPushButton *m_closeButton;
  QPushButton *m_minimizeButton;
  QTabBar *m_tabBar;

  QPoint m_dragPosition;
  bool m_dragging;
  bool m_resizing;
  Qt::Edges m_resizeEdge;

  QList<QString> m_tabResourceIds;
  QString m_currentResourceId;

  static constexpr int DEFAULT_WIDTH = 800;
  static constexpr int DEFAULT_HEIGHT = 600;
  static constexpr int RESIZE_MARGIN = 10;
};
