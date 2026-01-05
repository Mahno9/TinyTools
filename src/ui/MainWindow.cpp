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
    setupUI();
    setupWindowFlags();
    setupWebView();
    
    // Load saved position and size
    AppConfig config;
    if (config.load()) {
        resize(config.getWindowWidth(), config.getWindowHeight());
        move(config.getWindowX(), config.getWindowY());
        setWindowOpacity(config.getWindowOpacity() / 100.0);
        
        if (config.getAlwaysOnTop()) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        }
    }
}

MainWindow::~MainWindow() {
    // Save window position and size
    AppConfig config;
    if (config.load()) {
        config.setWindowWidth(width());
        config.setWindowHeight(height());
        config.setWindowX(x());
        config.setWindowY(y());
        config.save();
    }
}

void MainWindow::setupUI() {
    // Central widget
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    setCentralWidget(centralWidget);
    
    // Set initial size
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void MainWindow::setupWindowFlags() {
    // Frameless window with custom title bar behavior
    Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint;
    
    // Always on top (can be toggled)
    flags |= Qt::WindowStaysOnTopHint;
    
    setWindowFlags(flags);
    
    // Set window attributes
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
}

void MainWindow::setupWebView() {
    // Temporarily disable WebView to test basic app startup
    // WebView seems to be causing crash on startup
    QLabel* tempLabel = new QLabel("Yandex Translator\n\nWebView disabled for testing", this);
    tempLabel->setAlignment(Qt::AlignCenter);
    tempLabel->setStyleSheet("font-size: 18px; color: #333;");
    centralWidget()->layout()->addWidget(tempLabel);
    
    /*
    try {
        m_webView = new WebViewContainer(this);
        centralWidget()->layout()->addWidget(m_webView);
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
    */
}

void MainWindow::showAndActivate() {
    show();
    raise();
    activateWindow();
    
    // Ensure window is visible on screen
    QScreen* screen = QGuiApplication::screenAt(pos());
    if (!screen) {
        move(100, 100);
    }
}

void MainWindow::insertClipboardText() {
    if (!m_clipboardManager || !m_webView) return;
    
    QString text = m_clipboardManager->getText();
    if (!text.isEmpty()) {
        m_webView->insertText(text);
        qInfo() << "Inserted clipboard text (" << text.length() << " chars)";
    } else {
        qWarning() << "No text in clipboard";
        // Focus input field anyway
        m_webView->setFocus();
    }
}

void MainWindow::setOnlineStatus(bool online) {
    if (!m_webView) return;
    
    if (online) {
        m_webView->reloadTranslator();
    } else {
        // Show offline message in WebView
        m_webView->setHtml("<html><body style='background:#f0f0f0; "
                          "display:flex;justify-content:center;align-items:center;"
                          "height:100vh;'><h2>Network Offline</h2></body></html>");
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

void MainWindow::setOpacity(int value) {
    // Value is 0-100, convert to 0.0-1.0
    setWindowOpacity(value / 100.0);
}

void MainWindow::onSettingsRequested() {
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        qInfo() << "Settings saved";
    }
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Minimize to tray instead of closing
    hide();
    event->ignore();
}

void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            hide();
        }
    }
}
