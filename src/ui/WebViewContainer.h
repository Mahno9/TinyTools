#pragma once
#include "../models/WebResource.h"
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineView>

class WebViewContainer : public QWebEngineView {
  Q_OBJECT

public:
  explicit WebViewContainer(QWidget *parent = nullptr);

  void loadResource(const WebResource &resource);
  void insertText(const QString &text);
  void insertAltText(const QString &text);
  void reloadPage();
  bool isLoading() const;
  void applyWebViewTheme(bool darkTheme);

  QString getResourceId() const { return m_resourceId; }
  QUrl resourceUrl() const { return m_resourceUrl; }

signals:
  void pageLoaded(bool success);
  void zoomChanged(const QString &resourceId, double zoomFactor);

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void childEvent(QChildEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onLoadFinished(bool ok);
  void onRenderProcessTerminated(
      QWebEnginePage::RenderProcessTerminationStatus status, int exitCode);

private:
  void runOpenScript(bool alt, const QString &text);
  void showErrorPage(const QString &message);

  bool m_darkThemeEnabled = false;
  bool m_hasLoadedOk = false;
  int m_crashCount = 0;

  QString m_resourceId;
  QUrl m_resourceUrl;
};
