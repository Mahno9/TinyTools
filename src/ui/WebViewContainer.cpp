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
