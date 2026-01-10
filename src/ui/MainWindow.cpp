#include "MainWindow.h"
#include "WebViewContainer.h"
#include "../core/ClipboardManager.h"
#include "../models/AppConfig.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QLabel>
#include <QTimer>
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
        AppConfig* config = AppConfig::instance();
        if (config->load()) {
            qDebug() << "Configuration loaded successfully";
            
            int savedWidth = config->getWindowWidth();
            int savedHeight = config->getWindowHeight();
            int savedX = config->getWindowX();
            int savedY = config->getWindowY();
            double opacity = config->getWindowOpacity() / 100.0;
            bool alwaysOnTop = config->getAlwaysOnTop();
            
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
    AppConfig* config = AppConfig::instance();
    if (config->load()) {
        config->setWindowWidth(width());
        config->setWindowHeight(height());
        config->setWindowX(x());
        config->setWindowY(y());
        
        qDebug() << "Current size:" << width() << "x" << height();
        qDebug() << "Current position:" << x() << "," << y();
        
        if (config->save()) {
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
    
    // Create drag handle (title bar) for window movement
    m_dragHandle = new QWidget(this);
    m_dragHandle->setFixedHeight(30);
    m_dragHandle->setStyleSheet("background-color: #2b2b2b;");
    
    // Create horizontal layout for drag handle
    QHBoxLayout* dragHandleLayout = new QHBoxLayout(m_dragHandle);
    dragHandleLayout->setContentsMargins(0, 0, 5, 0);
    dragHandleLayout->setSpacing(0);
    
    // Create settings button (gear icon)
    m_settingsButton = new QPushButton("⚙", m_dragHandle);
    m_settingsButton->setFixedSize(30, 30);
    m_settingsButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: none;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3b3b3b;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #4b4b4b;"
        "}"
    );
    dragHandleLayout->addWidget(m_settingsButton);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    qDebug() << "Settings button created";
    
    // Add spacer to push control buttons to the right
    dragHandleLayout->addStretch();
    
    // Create minimize button
    m_minimizeButton = new QPushButton("_", m_dragHandle);
    m_minimizeButton->setFixedSize(30, 30);
    m_minimizeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: none;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3b3b3b;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #4b4b4b;"
        "}"
    );
    dragHandleLayout->addWidget(m_minimizeButton);
    connect(m_minimizeButton, &QPushButton::clicked, this, &MainWindow::onMinimizeButtonClicked);
    qDebug() << "Minimize button created";
    
    // Create close button
    m_closeButton = new QPushButton("✕", m_dragHandle);
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: none;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #ff5f5f;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #ff3f3f;"
        "}"
    );
    dragHandleLayout->addWidget(m_closeButton);
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseButtonClicked);
    qDebug() << "Close button created";
    
    layout->addWidget(m_dragHandle);
    qDebug() << "Drag handle created (30px height) with buttons";
    
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
        
        // Schedule startup theme application after WebView loads
        qDebug() << "Scheduling startup theme application...";
        QTimer::singleShot(2000, this, &MainWindow::applyStartupTheme);
        qDebug() << "Startup theme application scheduled (2 second delay)";
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

void MainWindow::applyWebViewTheme(bool darkTheme) {
    qDebug() << "MainWindow::applyWebViewTheme() - ENTRY";
    qDebug() << "Applying WebView theme:" << (darkTheme ? "dark" : "light");
    
    if (!m_webView) {
        qWarning() << "Cannot apply WebView theme - m_webView is null";
        qDebug() << "MainWindow::applyWebViewTheme() - EXIT";
        return;
    }
    
    m_webView->applyWebViewTheme(darkTheme);
    qInfo() << "WebView theme applied:" << (darkTheme ? "dark" : "light");
    
    qDebug() << "MainWindow::applyWebViewTheme() - EXIT";
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
    
    AppConfig* config = AppConfig::instance();
    
    // Apply window settings
    double opacity = config->getWindowOpacity() / 100.0;
    qDebug() << "Applying opacity:" << opacity;
    setWindowOpacity(opacity);
    
    bool alwaysOnTop = config->getAlwaysOnTop();
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
    AppConfig* config = AppConfig::instance();
    if (config->load()) {
        bool autoTranslate = config->getAutoTranslate();
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
    qDebug() << "Local y position:" << event->pos().y();
    
    // Only allow dragging when mouse is over the drag handle (top 30px)
    if (event->button() == Qt::LeftButton && event->pos().y() <= 30) {
        qDebug() << "Left button pressed on drag handle - starting drag";
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

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
    QMainWindow::mouseReleaseEvent(event);
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

void MainWindow::applyStartupTheme() {
    qDebug() << "MainWindow::applyStartupTheme() - ENTRY";
    
    // Load config to get dark theme setting
    AppConfig* config = AppConfig::instance();
    if (!config->load()) {
        qWarning() << "Failed to load config for startup theme - using default (light theme)";
        qDebug() << "MainWindow::applyStartupTheme() - EXIT";
        return;
    }
    
    bool darkTheme = config->getDarkTheme();
    qDebug() << "Startup dark theme setting:" << (darkTheme ? "dark" : "light");
    
    // Apply WebView theme
    if (m_webView) {
        qDebug() << "Applying WebView theme on startup...";
        m_webView->applyWebViewTheme(darkTheme);
        qInfo() << "Startup WebView theme applied:" << (darkTheme ? "dark" : "light");
    } else {
        qWarning() << "Cannot apply WebView theme - m_webView is null";
    }
    
    qDebug() << "MainWindow::applyStartupTheme() - EXIT";
}

void MainWindow::onCloseButtonClicked() {
    qDebug() << "MainWindow::onCloseButtonClicked() - ENTRY";
    
    // Hide the window (same behavior as closeEvent)
    hide();
    qDebug() << "Window hidden (close button clicked)";
    
    qDebug() << "MainWindow::onCloseButtonClicked() - EXIT";
}

void MainWindow::onMinimizeButtonClicked() {
    qDebug() << "MainWindow::onMinimizeButtonClicked() - ENTRY";
    
    // Minimize the window
    showMinimized();
    qDebug() << "Window minimized";
    
    qDebug() << "MainWindow::onMinimizeButtonClicked() - EXIT";
}

void MainWindow::onSettingsButtonClicked() {
    qDebug() << "MainWindow::onSettingsButtonClicked() - ENTRY";
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
    
    qDebug() << "MainWindow::onSettingsButtonClicked() - EXIT";
}
