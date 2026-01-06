#pragma once
#include <QMainWindow>
#include <QPointer>

class WebViewContainer;
class ClipboardManager;
class QSystemTrayIcon;
class QMouseEvent;
class QCloseEvent;
class QEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(ClipboardManager* clipboardManager, QWidget* parent = nullptr);
    ~MainWindow();
    
    void showAndActivate();
    void insertClipboardText();
    void setOnlineStatus(bool online);
    void toggleAlwaysOnTop();
    
public slots:
    void onSettingsRequested();
    
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    
private slots:
   void setOpacity(int value);
   void onClipboardChanged(const QString& text);
   
private:
    void setupUI();
    void setupWindowFlags();
    void setupWebView();
    void applySettings();
    
    QPointer<WebViewContainer> m_webView;
    QPointer<ClipboardManager> m_clipboardManager;
    
    QPoint m_dragPosition;
    bool m_dragging;
    
    static constexpr int DEFAULT_WIDTH = 800;
    static constexpr int DEFAULT_HEIGHT = 600;
};
