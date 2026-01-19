#pragma once
#include <QMainWindow>
#include <QPointer>

class WebViewContainer;
class ClipboardManager;
class QSystemTrayIcon;
class QPushButton;
class QMouseEvent;
class QCloseEvent;
class QTabBar;
class QEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(ClipboardManager* clipboardManager, QWidget* parent = nullptr);
    ~MainWindow();
    
    void showAndActivate();
    void insertClipboardText(bool useAltScript = false);
    void setOnlineStatus(bool online);
    void toggleAlwaysOnTop();
    void applyWebViewTheme(bool darkTheme);
    void switchToResource(int index);
    void switchToResourceById(const QString& id);
    
public slots:
    void onSettingsRequested();
    void applyStartupTheme();
    void refreshResources();
    
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    
private slots:
   void setOpacity(int value);
   void onClipboardChanged(const QString& text);
   void onCloseButtonClicked();
   void onMinimizeButtonClicked();
   void onSettingsButtonClicked();
   void onTabChanged(int index);
   
private:
    void setupUI();
    void setupWindowFlags();
    void setupWebView();
    void applySettings();
    void loadCurrentResource();
    
    QPointer<WebViewContainer> m_webView;
    QPointer<ClipboardManager> m_clipboardManager;
    
    QWidget* m_dragHandle;
    QPushButton* m_settingsButton;
    QPushButton* m_closeButton;
    QPushButton* m_minimizeButton;
    QTabBar* m_tabBar;
    
    QPoint m_dragPosition;
    bool m_dragging;
    
    QList<QString> m_tabResourceIds;
    QString m_currentResourceId;
    
    static constexpr int DEFAULT_WIDTH = 800;
    static constexpr int DEFAULT_HEIGHT = 600;
};
