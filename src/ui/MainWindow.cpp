#include "MainWindow.h"
#include "WebViewContainer.h"
#include "../core/ClipboardManager.h"
#include "../models/AppConfig.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(ClipboardManager* clipboardManager, QWidget* parent)
    : QMainWindow(parent)
    , m_clipboardManager(clipboardManager)
    , m_dragging(false)
{
    qDebug() << "MainWindow::MainWindow() - ENTRY";
    qDebug() << "Creating MainWindow with clipboardManager:" << (clipboardManager ? "yes" : "no");
    qDebug() << "Parent widget:" << (parent ? "yes" : "no");
    
    try {
        qDebug() << "Step 1: Setting up UI...";
        setupUI();
        qDebug() << "Step 1 complete: UI setup finished";
        
        qDebug() << "Step 2: Setting up window flags...";
        setupWindowFlags();
        qDebug() << "Step 2 complete: Window flags set";
        
        qDebug() << "Step 3: Setting up WebView...";
        setupWebView();
        qDebug() << "Step 3 complete: WebView setup finished";
        
        qDebug() << "Step 4: Loading saved window configuration...";
        AppConfig config;
        if (config.load()) {
            qDebug() << "Configuration loaded successfully";
            
            int savedWidth = config.getWindowWidth();
            int savedHeight = config.getWindowHeight();
            int savedX = config.getWindowX();
            int savedY = config.getWindowY();
            double opacity = config.getWindowOpacity() / 100.0;
            bool alwaysOnTop = config.getAlwaysOnTop();
            
            qDebug() << "Saved size:" << savedWidth << "x" << savedHeight;
            qDebug() << "Saved position:" << savedX << "," << savedY;
            qDebug() << "Saved opacity:" << opacity;
            qDebug() << "Always on top:" << (alwaysOnTop ? "yes" : "no");
            
            resize(savedWidth, savedHeight);
            qDebug() << "Window resized to:" << width() << "x" << height();
            
            move(savedX, savedY);
            qDebug() << "Window moved to:" << x() << "," << y();
            
            setWindowOpacity(opacity);
            qDebug() << "Window opacity set to:" << windowOpacity();
            
            if (alwaysOnTop) {
                qDebug() << "Setting always-on-top flag";
                setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
                qDebug() << "Always-on-top flag set";
            }
            
            qDebug() << "Step 4 complete: Window configuration loaded and applied";
        } else {
            qWarning() << "Failed to load configuration - using default values";
            qDebug() << "Default size:" << DEFAULT_WIDTH << "x" << DEFAULT_HEIGHT;
        }
        
        qDebug() << "MainWindow constructed successfully";
        qDebug() << "MainWindow::MainWindow() - EXIT";
    } catch (const std::exception& e) {
        qCritical() << "MainWindow::MainWindow() - EXCEPTION:" << e.what();
        qCritical() << "MainWindow::MainWindow() - EXIT with error";
        throw;
    } catch (...) {
        qCritical() << "MainWindow::MainWindow() - EXCEPTION: Unknown error";
        qCritical() << "MainWindow::MainWindow() - EXIT with error";
        throw;
    }
}

MainWindow::~MainWindow() {
    qDebug() << "MainWindow::~MainWindow() - ENTRY";
    qDebug() << "Destroying MainWindow";
    
    qDebug() << "Saving window position and size...";
    AppConfig config;
    if (config.load()) {
        config.setWindowWidth(width());
        config.setWindowHeight(height());
        config.setWindowX(x());
        config.setWindowY(y());
        
        qDebug() << "Current size:" << width() << "x" << height();
        qDebug() << "Current position:" << x() << "," << y();
        
        if (config.save()) {
            qDebug() << "Window configuration saved successfully";
        } else {
            qWarning() << "Failed to save window configuration";
        }
    } else {
        qWarning() << "Failed to load config for saving - window state not saved";
    }
    
    qDebug() << "MainWindow destroyed";
    qDebug() << "MainWindow::~MainWindow() - EXIT";
}

void MainWindow::setupUI() {
    qDebug() << "MainWindow::setupUI() - ENTRY";
    qDebug() << "Creating central widget and layout...";
    
    // Central widget
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    setCentralWidget(centralWidget);
    qDebug() << "Central widget set";
    
    // Set initial size
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    qDebug() << "Initial window size set to:" << DEFAULT_WIDTH << "x" << DEFAULT_HEIGHT;
    
    qDebug() << "MainWindow::setupUI() - EXIT";
}

void MainWindow::setupWindowFlags() {
    qDebug() << "MainWindow::setupWindowFlags() - ENTRY";
    
    // Frameless window with custom title bar behavior
    Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint;
    qDebug() << "Base flags: Window + FramelessWindowHint";
    
    // Always on top (can be toggled)
    flags |= Qt::WindowStaysOnTopHint;
    qDebug() << "Added WindowStaysOnTopHint";
    
    setWindowFlags(flags);
    qDebug() << "Window flags applied";
    
    // Set window attributes
    setAttribute(Qt::WA_TranslucentBackground);
    qDebug() << "Attribute WA_TranslucentBackground set";
    
    setAttribute(Qt::WA_NoSystemBackground);
    qDebug() << "Attribute WA_NoSystemBackground set";
    
    qDebug() << "MainWindow::setupWindowFlags() - EXIT";
}

void MainWindow::setupWebView() {
    qDebug() << "MainWindow::setupWebView() - ENTRY";
    qDebug() << "Initializing WebView container...";
    
    try {
        m_webView = new WebViewContainer(this);
        centralWidget()->layout()->addWidget(m_webView);
        qDebug() << "WebView initialized successfully";
        
        // Connect clipboard manager signal for auto-translate
        if (m_clipboardManager) {
            qDebug() << "Connecting clipboard manager signal...";
            connect(m_clipboardManager, &ClipboardManager::clipboardChanged,
                    this, &MainWindow::onClipboardChanged);
            qDebug() << "Clipboard manager signal connected";
        } else {
            qWarning() << "Cannot connect clipboard manager - m_clipboardManager is null";
        }
    } catch (const std::exception& e) {
        qCritical() << "Failed to initialize WebView:" << e.what();
        // Show error message
        QLabel* errorLabel = new QLabel("WebView initialization failed.\nPlease check Qt WebEngine is properly installed.", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        centralWidget()->layout()->addWidget(errorLabel);
    } catch (...) {
        qCritical() << "Failed to initialize WebView: unknown error";
        QLabel* errorLabel = new QLabel("WebView initialization failed.\nPlease check Qt WebEngine is properly installed.", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        centralWidget()->layout()->addWidget(errorLabel);
    }
    
    qDebug() << "MainWindow::setupWebView() - EXIT";
}

void MainWindow::showAndActivate() {
    qDebug() << "MainWindow::showAndActivate() - ENTRY";
    qDebug() << "Current visibility:" << (isVisible() ? "visible" : "hidden");
    qDebug() << "Window position:" << x() << "," << y();
    qDebug() << "Window size:" << width() << "x" << height();
    
    show();
    qDebug() << "Window shown";
    
    raise();
    qDebug() << "Window raised";
    
    activateWindow();
    qDebug() << "Window activated";
    
    // Ensure window is visible on screen
    QScreen* screen = QGuiApplication::screenAt(pos());
    if (!screen) {
        qWarning() << "Window not on any screen, moving to (100, 100)";
        move(100, 100);
    } else {
        qDebug() << "Window is on screen";
    }
    
    qDebug() << "Window now visible:" << (isVisible() ? "yes" : "no");
    qDebug() << "MainWindow::showAndActivate() - EXIT";
}

void MainWindow::insertClipboardText() {
    qDebug() << "MainWindow::insertClipboardText() - ENTRY";
    
    if (!m_clipboardManager) {
        qWarning() << "Cannot insert clipboard text - m_clipboardManager is null";
        qDebug() << "MainWindow::insertClipboardText() - EXIT";
        return;
    }
    
    if (!m_webView) {
        qWarning() << "Cannot insert clipboard text - m_webView is null";
        qDebug() << "MainWindow::insertClipboardText() - EXIT";
        return;
    }
    
    QString text = m_clipboardManager->getText();
    qDebug() << "Clipboard text length:" << text.length() << "characters";
    
    if (!text.isEmpty()) {
        m_webView->insertText(text);
        qInfo() << "Inserted clipboard text (" << text.length() << " chars)";
    } else {
        qWarning() << "No text in clipboard";
        // Focus input field anyway
        m_webView->setFocus();
        qDebug() << "WebView focused despite empty clipboard";
    }
    
    qDebug() << "MainWindow::insertClipboardText() - EXIT";
}

void MainWindow::setOnlineStatus(bool online) {
    qDebug() << "MainWindow::setOnlineStatus() - ENTRY";
    qDebug() << "Online status:" << (online ? "ONLINE" : "OFFLINE");
    
    if (!m_webView) {
        qWarning() << "Cannot set online status - m_webView is null";
        qDebug() << "MainWindow::setOnlineStatus() - EXIT";
        return;
    }
    
    if (online) {
        qDebug() << "Reloading translator...";
        m_webView->reloadTranslator();
        qDebug() << "Translator reloaded";
    } else {
        qDebug() << "Showing offline message in WebView...";
        // Show offline message in WebView
        m_webView->setHtml("<html><body style='background:#f0f0f0; "
                          "display:flex;justify-content:center;align-items:center;"
                          "height:100vh;'><h2>Network Offline</h2></body></html>");
        qDebug() << "Offline message displayed";
    }
    
    qDebug() << "MainWindow::setOnlineStatus() - EXIT";
}

void MainWindow::toggleAlwaysOnTop() {
    qDebug() << "MainWindow::toggleAlwaysOnTop() - ENTRY";
    
    Qt::WindowFlags flags = windowFlags();
    bool currentlyOnTop = (flags & Qt::WindowStaysOnTopHint);
    qDebug() << "Currently always on top:" << (currentlyOnTop ? "yes" : "no");
    
    if (currentlyOnTop) {
        flags &= ~Qt::WindowStaysOnTopHint;
        qDebug() << "Removing WindowStaysOnTopHint";
    } else {
        flags |= Qt::WindowStaysOnTopHint;
        qDebug() << "Adding WindowStaysOnTopHint";
    }
    
    setWindowFlags(flags);
    show();
    qDebug() << "Window flags updated and window reshown";
    qDebug() << "Now always on top:" << ((flags & Qt::WindowStaysOnTopHint) ? "yes" : "no");
    
    qDebug() << "MainWindow::toggleAlwaysOnTop() - EXIT";
}

void MainWindow::setOpacity(int value) {
    qDebug() << "MainWindow::setOpacity() - ENTRY";
    qDebug() << "Opacity value:" << value << " (0-100)";
    
    // Value is 0-100, convert to 0.0-1.0
    double opacity = value / 100.0;
    setWindowOpacity(opacity);
    qDebug() << "Window opacity set to:" << opacity;
    
    qDebug() << "MainWindow::setOpacity() - EXIT";
}

void MainWindow::onSettingsRequested() {
    qDebug() << "MainWindow::onSettingsRequested() - ENTRY";
    qDebug() << "Opening settings dialog...";
    
    SettingsDialog dialog(this);
    int result = dialog.exec();
    
    if (result == QDialog::Accepted) {
        qInfo() << "Settings saved";
        qDebug() << "Settings dialog accepted";
        
        // Apply settings changes
        qDebug() << "Applying settings changes...";
        applySettings();
    } else {
        qDebug() << "Settings dialog rejected/cancelled";
    }
    
    qDebug() << "MainWindow::onSettingsRequested() - EXIT";
}

void MainWindow::applySettings() {
    qDebug() << "MainWindow::applySettings() - ENTRY";
    
    AppConfig config;
    if (!config.load()) {
        qWarning() << "Failed to load config for applying settings";
        return;
    }
    
    // Apply window settings
    double opacity = config.getWindowOpacity() / 100.0;
    qDebug() << "Applying opacity:" << opacity;
    setWindowOpacity(opacity);
    
    bool alwaysOnTop = config.getAlwaysOnTop();
    Qt::WindowFlags flags = windowFlags();
    bool currentlyOnTop = (flags & Qt::WindowStaysOnTopHint);
    
    if (alwaysOnTop != currentlyOnTop) {
        qDebug() << "Applying always-on-top:" << (alwaysOnTop ? "yes" : "no");
        if (alwaysOnTop) {
            flags |= Qt::WindowStaysOnTopHint;
        } else {
            flags &= ~Qt::WindowStaysOnTopHint;
        }
        setWindowFlags(flags);
        show();
    }
    
    qDebug() << "Settings applied successfully";
    qDebug() << "MainWindow::applySettings() - EXIT";
}

void MainWindow::onClipboardChanged(const QString& text) {
    qDebug() << "MainWindow::onClipboardChanged() - ENTRY";
    qDebug() << "Clipboard text length:" << text.length() << "characters";
    
    // Check if auto-translate is enabled
    AppConfig config;
    if (config.load()) {
        bool autoTranslate = config.getAutoTranslate();
        qDebug() << "Auto-translate enabled:" << (autoTranslate ? "yes" : "no");
        
        if (autoTranslate && !text.isEmpty()) {
            qDebug() << "Auto-translate triggered - showing window and inserting text";
            
            // Show window
            if (!isVisible()) {
                showAndActivate();
            }
            
            // Insert text into WebView
            if (m_webView && !m_webView->isLoading()) {
                m_webView->insertText(text);
                qInfo() << "Auto-translated clipboard text (" << text.length() << " chars)";
            } else {
                qWarning() << "Cannot auto-translate - WebView not ready or loading";
            }
        } else {
            qDebug() << "Auto-translate not triggered (disabled or empty text)";
        }
    } else {
        qWarning() << "Failed to load config for auto-translate check";
    }
    
    qDebug() << "MainWindow::onClipboardChanged() - EXIT";
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    qDebug() << "MainWindow::mousePressEvent() - ENTRY";
    qDebug() << "Button pressed:" << event->button();
    qDebug() << "Mouse position:" << event->globalPos();
    
    if (event->button() == Qt::LeftButton) {
        qDebug() << "Left button pressed - starting drag";
        m_dragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        qDebug() << "Drag position set to:" << m_dragPosition;
        event->accept();
    }
    
    qDebug() << "MainWindow::mousePressEvent() - EXIT";
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && event->buttons() & Qt::LeftButton) {
        qDebug() << "Dragging window to:" << (event->globalPos() - m_dragPosition);
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    qDebug() << "MainWindow::closeEvent() - ENTRY";
    qDebug() << "Close event received - minimizing to tray instead of closing";
    
    // Minimize to tray instead of closing
    hide();
    qDebug() << "Window hidden";
    
    event->ignore();
    qDebug() << "Close event ignored";
    
    qDebug() << "MainWindow::closeEvent() - EXIT";
}

void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        qDebug() << "Window state changed";
        qDebug() << "Current state:" << windowState();
        qDebug() << "Is minimized:" << (isMinimized() ? "yes" : "no");
        
        if (isMinimized()) {
            qDebug() << "Window minimized - hiding to tray";
            hide();
        }
    }
}
