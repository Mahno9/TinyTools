#include "WebViewContainer.h"
#include <QContextMenuEvent>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QTimer>
#include <QDebug>

const char* WebViewContainer::TRANSLATOR_URL = "https://translate.yandex.ru/";
const char* WebViewContainer::INPUT_SELECTOR = "textarea[aria-label*='text']";

WebViewContainer::WebViewContainer(QWidget* parent)
    : QWebEngineView(parent)
    , m_darkThemeEnabled(false)
    , m_darkThemeApplied(false)
{
    // Configure page
    QWebEnginePage* page = new QWebEnginePage(this);
    setPage(page);
    
    // Configure page settings for performance
    QWebEngineSettings* settings = page->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    
    // Connect signals
    connect(page, &QWebEnginePage::loadFinished,
            this, &WebViewContainer::onLoadFinished);
    connect(page, &QWebEnginePage::loadProgress,
            this, &WebViewContainer::onLoadProgress);
    connect(page, &QWebEnginePage::renderProcessTerminated,
            this, &WebViewContainer::onRenderProcessTerminated);
    
    // Load translator page
    load(QUrl(TRANSLATOR_URL));
}

void WebViewContainer::insertText(const QString& text) {
    if (isLoading()) {
        qWarning() << "Cannot insert text: page is loading";
        return;
    }
    
    findAndInsertInInputField(text);
}

void WebViewContainer::reloadTranslator() {
    load(QUrl(TRANSLATOR_URL));
}

bool WebViewContainer::isLoading() const {
    return page() && page()->isLoading();
}

void WebViewContainer::onLoadFinished(bool ok) {
    if (ok) {
        qDebug() << "Page loaded successfully";
        emit pageLoaded(true);
        
        // Reapply dark theme if enabled
        if (m_darkThemeEnabled) {
            m_darkThemeApplied = false; // Reset to force reapplication
            applyWebViewTheme(true);
        }
    } else {
        qWarning() << "Page load failed";
        emit loadError("Failed to load translator page");
        emit pageLoaded(false);
    }
}

void WebViewContainer::onLoadProgress(int progress) {
    if (progress % 25 == 0) {
        qDebug() << "Loading progress:" << progress << "%";
    }
}

void WebViewContainer::onRenderProcessTerminated(
    QWebEnginePage::RenderProcessTerminationStatus status, 
    int exitCode)
{
    Q_UNUSED(exitCode);
    
    QString reason;
    switch (status) {
        case QWebEnginePage::NormalTerminationStatus:
            reason = "Normal termination";
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            reason = "Abnormal termination";
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            reason = "Render process crashed";
            break;
        case QWebEnginePage::KilledTerminationStatus:
            reason = "Render process killed";
            break;
    }
    
    qCritical() << "Render process terminated:" << reason;
    emit loadError(reason);
    
    // Attempt to reload
    QTimer::singleShot(1000, this, &WebViewContainer::reloadTranslator);
}

void WebViewContainer::findAndInsertInInputField(const QString& text) {
    // Escape text for JavaScript
    QString escapedText = text.toHtmlEscaped();
    
    QString script = QString(R"(
        (function() {
            // Try multiple selectors to find the input field
            const selectors = [
                'textarea[aria-label*="text" i]',
                'textarea[placeholder*="text" i]',
                'textarea[data-testid*="input" i]',
                '.input textarea',
                '#text-input',
                'textarea'
            ];
            
            let inputElement = null;
            
            for (const selector of selectors) {
                inputElement = document.querySelector(selector);
                if (inputElement) {
                    console.log('Found input with selector:', selector);
                    break;
                }
            }
            
            if (inputElement) {
                // Focus the input
                inputElement.focus();
                
                // Set the value
                inputElement.value = '%1';
                
                // Dispatch input event to trigger translation
                const event = new Event('input', { bubbles: true });
                inputElement.dispatchEvent(event);
                
                // Dispatch change event
                const changeEvent = new Event('change', { bubbles: true });
                inputElement.dispatchEvent(changeEvent);
                
                return { success: true, selector: selectors.join(', ') };
            } else {
                console.error('Could not find input field');
                return { success: false, error: 'Input element not found' };
            }
        })();
    )").arg(escapedText);
    
    injectJavaScript(script);
}

void WebViewContainer::injectJavaScript(const QString& script) {
    if (!page()) return;
    
    page()->runJavaScript(script, [this](const QVariant& result) {
        QVariantMap resultMap = result.toMap();
        bool success = resultMap.value("success", false).toBool();
        
        if (success) {
            qDebug() << "Text inserted successfully";
        } else {
            QString error = resultMap.value("error", "Unknown error").toString();
            qWarning() << "Failed to insert text:" << error;
        }
    });
}

void WebViewContainer::waitForPageLoad() {
    // This method can be used to wait for page load before inserting text
    // Currently, we insert text immediately and rely on the page being loaded
    qDebug() << "Waiting for page load...";
}

void WebViewContainer::contextMenuEvent(QContextMenuEvent* event) {
    // Disable context menu for cleaner UI
    event->ignore();
}

void WebViewContainer::applyWebViewTheme(bool darkTheme) {
    m_darkThemeEnabled = darkTheme;
    
    if (!darkTheme) {
        // Remove dark theme by reloading the page
        if (m_darkThemeApplied) {
            qDebug() << "Removing dark theme";
            reloadTranslator();
            m_darkThemeApplied = false;
        }
        return;
    }
    
    // Apply dark theme
    if (m_darkThemeApplied) {
        qDebug() << "Dark theme already applied";
        return;
    }
    
    qDebug() << "Applying dark theme to WebView";
    
    QString script = QString(R"(
        (function() {
            try {
                // Create a unique ID for our style element
                const styleId = 'yandex-translator-dark-theme';
                
                // Remove existing dark theme style if present
                const existingStyle = document.getElementById(styleId);
                if (existingStyle) {
                    existingStyle.remove();
                }
                
                // Create new style element
                const style = document.createElement('style');
                style.id = styleId;
                style.innerHTML = `
                    /* Background and text colors */
                    html, body {
                        background-color: #2b2b2b !important;
                        color: #ffffff !important;
                    }
                    
                    /* Container elements */
                    .container, .content, .wrapper, .translation,
                    .source-text, .result-text, .main, .app-container,
                    .translator-container, .page-container {
                        background-color: #2b2b2b !important;
                        color: #ffffff !important;
                    }
                    
                    /* Input areas */
                    textarea, input[type="text"], input[type="search"] {
                        background-color: #3c3f41 !important;
                        color: #ffffff !important;
                        border-color: #555 !important;
                    }
                    
                    textarea::placeholder, input::placeholder {
                        color: #aaaaaa !important;
                    }
                    
                    /* Buttons */
                    button, .button, [role="button"] {
                        background-color: #5896d8 !important;
                        color: #ffffff !important;
                    }
                    
                    button:hover, .button:hover, [role="button"]:hover {
                        background-color: #4a7eb5 !important;
                        color: #ffffff !important;
                    }
                    
                    /* Header and navigation */
                    header, .header, .footer, .nav, .navigation,
                    .sidebar, .panel {
                        background-color: #3c3f41 !important;
                        color: #ffffff !important;
                    }
                    
                    /* Links */
                    a {
                        color: #68a6e8 !important;
                    }
                    
                    a:hover {
                        color: #88b8f8 !important;
                    }
                    
                    /* Cards and panels */
                    .card, .panel, .box, .segment {
                        background-color: #3c3f41 !important;
                        color: #ffffff !important;
                        border-color: #555 !important;
                    }
                    
                    /* Lists and items */
                    .list-item, .option, .suggestion {
                        background-color: #3c3f41 !important;
                        color: #ffffff !important;
                    }
                    
                    .list-item:hover, .option:hover, .suggestion:hover {
                        background-color: #4a4d4f !important;
                    }
                    
                    /* Dropdowns and selects */
                    select, .select {
                        background-color: #3c3f41 !important;
                        color: #ffffff !important;
                        border-color: #555 !important;
                    }
                    
                    /* Icons */
                    .icon, [class*="icon"] {
                        color: #ffffff !important;
                    }
                    
                    /* Special Yandex Translator specific elements */
                    .input-area, .output-area, .translation-input,
                    .translation-output, .text-area {
                        background-color: #3c3f41 !important;
                        color: #ffffff !important;
                    }
                    
                    /* Shadows and borders */
                    * {
                        box-shadow: none !important;
                    }
                    
                    /* Hide scrollbars */
                    ::-webkit-scrollbar {
                        display: none;
                    }
                    body {
                        overflow: hidden;
                    }
                `;
                
                document.head.appendChild(style);
                console.log('Dark theme applied successfully');
                return { success: true };
            } catch (error) {
                console.error('Error applying dark theme:', error);
                return { success: false, error: error.message };
            }
        })();
    )");
    
    injectJavaScript(script);
    m_darkThemeApplied = true;
}
