#pragma once
#include <QWebEngineView>
#include <QWebEnginePage>
#include "../models/WebResource.h"

class WebViewContainer : public QWebEngineView {
    Q_OBJECT
    
public:
    explicit WebViewContainer(QWidget* parent = nullptr);
    
    void insertText(const QString& text);
    void insertAltText(const QString& text);
    void reloadTranslator();
    bool isLoading() const;
    void applyWebViewTheme(bool darkTheme);
    
signals:
    void pageLoaded(bool success);
    void loadError(const QString& error);
    
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    
private slots:
    void onLoadFinished(bool ok);
    void onLoadProgress(int progress);
    void onRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status, 
                                   int exitCode);
    
public:
    void loadResource(const WebResource& resource);
    void executeScript(const QString& script);
    
private:
    
private:
    void injectJavaScript(const QString& script);
    void waitForPageLoad();
    
    // Removed hardcoded TRANSLATOR_URL and INPUT_SELECTOR
    
    bool m_darkThemeEnabled;
    bool m_darkThemeApplied;
    
    QString m_openScript;
    QString m_altOpenScript;
};
