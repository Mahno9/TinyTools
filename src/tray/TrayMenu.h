#pragma once
#include <QMenu>

class MainWindow;
class TrayMenu : public QMenu {
    Q_OBJECT
    
public:
    explicit TrayMenu(QWidget* parent = nullptr);
    ~TrayMenu();
    
    void setMainWindow(MainWindow* mainWindow);
    
private slots:
    void onShowWindow();
    void onHideWindow();
    void onToggleAlwaysOnTop();
    void onOpenSettings();
    void onReload();
    void onCheckForUpdates();
    void onQuit();
    
private:
    void setupMenu();
    
    MainWindow* m_mainWindow;
};
