#pragma once
#include <QObject>
#include <QClipboard>

class ClipboardManager : public QObject {
    Q_OBJECT
    
public:
    explicit ClipboardManager(QObject* parent = nullptr);
    
    QString getText() const;
    bool hasText() const;
    void setText(const QString& text);
    
    static bool isValidText(const QString& text);
    static QString trimText(const QString& text, int maxLength = 10000);
    
signals:
    void clipboardChanged(const QString& text);
    
private slots:
    void onClipboardChanged(QClipboard::Mode mode);
    
private:
    QClipboard* m_clipboard;
    QString m_lastText;
};
