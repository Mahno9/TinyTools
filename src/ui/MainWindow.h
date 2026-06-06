#pragma once
#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTimer>

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
  bool eventFilter(QObject *watched, QEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;
  void hideEvent(QHideEvent *event) override;
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
  void onTabContextMenuRequested(const QPoint &pos);
  void onRefreshTriggered();

private:
  void setupUI();
  void setupWindowFlags();
  void setupWebView();
  void applySettings();
  void loadCurrentResource();
  void reloadResource(const QString &resourceId = QString());

  QPointer<ClipboardManager> m_clipboardManager;

  QWidget *m_dragHandle = nullptr;
  QStackedWidget *m_stackedWidget = nullptr;
  QMap<QString, WebViewContainer *> m_resourceViews;
  WebViewContainer *m_webView = nullptr;

  QPushButton *m_settingsButton = nullptr;
  QPushButton *m_closeButton = nullptr;
  QPushButton *m_minimizeButton = nullptr;
  QTabBar *m_tabBar = nullptr;

  QPoint m_dragPosition;
  bool m_dragging;
  bool m_resizing;
  Qt::Edges m_resizeEdge;

  QList<QString> m_tabResourceIds;
  QString m_currentResourceId;

  QAction *m_refreshAction = nullptr;
  QTimer *m_zoomSaveTimer = nullptr;

  static constexpr int DEFAULT_WIDTH = 800;
  static constexpr int DEFAULT_HEIGHT = 600;
  static constexpr int RESIZE_MARGIN = 10;
};
