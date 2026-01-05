#pragma once
#include <QWebEngineView>
#include <QWebEnginePage>

class WebViewContainer : public QWebEngineView {
    Q_OBJECT
    
public:
    explicit WebViewContainer(QWidget* parent = nullptr);
    
    void insertText(const QString& text);
    void reloadTranslator();
    bool isLoading() const;
    
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
    
private:
    void injectJavaScript(const QString& script);
    void waitForPageLoad();
    void findAndInsertInInputField(const QString& text);
    
    static const char* TRANSLATOR_URL;
    static const char* INPUT_SELECTOR;
};
