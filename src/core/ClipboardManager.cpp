#include "ClipboardManager.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>

ClipboardManager::ClipboardManager(QObject* parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
{
    connect(m_clipboard, &QClipboard::dataChanged,
            this, [this]() { onClipboardChanged(QClipboard::Clipboard); });
}

QString ClipboardManager::getText() const {
    if (!m_clipboard) return QString();
    
    QString text = m_clipboard->text(QClipboard::Clipboard);
    return isValidText(text) ? text : QString();
}

bool ClipboardManager::hasText() const {
    return !getText().isEmpty();
}

void ClipboardManager::setText(const QString& text) {
    if (m_clipboard) {
        m_clipboard->setText(text, QClipboard::Clipboard);
    }
}

bool ClipboardManager::isValidText(const QString& text) {
    // Filter out empty, whitespace-only, or extremely long text
    if (text.isEmpty()) return false;
    if (text.trimmed().isEmpty()) return false;
    if (text.length() > 100000) return false; // 100KB limit
    
    return true;
}

QString ClipboardManager::trimText(const QString& text, int maxLength) {
    if (text.length() <= maxLength) return text;
    return text.left(maxLength) + "...";
}

void ClipboardManager::onClipboardChanged(QClipboard::Mode mode) {
    if (mode != QClipboard::Clipboard) return;
    
    QString text = getText();
    if (text != m_lastText && isValidText(text)) {
        m_lastText = text;
        emit clipboardChanged(text);
    }
}
