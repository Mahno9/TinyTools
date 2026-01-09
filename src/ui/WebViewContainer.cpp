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
    settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);
    
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
    
    // Determine which button to click based on theme preference
    QString ariaLabel = darkTheme ? QString("Тёмная") : QString("Светлая");
    QString themeName = darkTheme ? QString("dark") : QString("light");
    
    qDebug() << QString("Applying %1 theme to WebView via button click").arg(themeName);
    
    QString script = QString(R"(
        (function() {
            try {
                // Find the theme button using the selector
                const ariaLabel = '%1';
                const selector = `.choiceGroup-item[aria-label="${ariaLabel}"]`;
                const button = document.querySelector(selector);
                
                if (!button) {
                    console.error('Theme button not found with selector:', selector);
                    
                    // Try alternative approach: look for any theme toggle button
                    const themeButtons = document.querySelectorAll('[class*="choiceGroup-item"]');
                    console.log('Found', themeButtons.length, 'potential theme buttons');
                    
                    for (let btn of themeButtons) {
                        const label = btn.getAttribute('aria-label');
                        if (label && label.toLowerCase().includes(ariaLabel.toLowerCase())) {
                            console.log('Found theme button via alternative search');
                            btn.click();
                            return { success: true, method: 'alternative' };
                        }
                    }
                    
                    return { success: false, error: 'Theme button not found' };
                }
                
                // Click the button to toggle the theme
                button.click();
                console.log('Theme button clicked successfully');
                return { success: true, selector: selector };
                
            } catch (error) {
                console.error('Error applying theme:', error);
                return { success: false, error: error.message };
            }
        })();
    )").arg(ariaLabel);
    
    injectJavaScript(script);
    m_darkThemeApplied = darkTheme;
}
