#include "ClipboardManager.h"
#include <QApplication>
#include <QClipboard>
#include <QPixmap>
#include <QMimeData>
#include <QDebug>

ClipboardManager::ClipboardManager(QObject* parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
{
    qDebug() << "ClipboardManager::ClipboardManager() - ENTRY";
    qDebug() << "Creating ClipboardManager with parent:" << (parent ? "yes" : "no");
    
    qDebug() << "Getting clipboard instance...";
    if (m_clipboard) {
        qDebug() << "Clipboard instance obtained successfully";
    } else {
        qWarning() << "Failed to get clipboard instance";
        qCritical() << "ClipboardManager created without valid clipboard - may not function properly";
    }
    
    qDebug() << "Connecting clipboard change signal...";
    connect(m_clipboard, &QClipboard::dataChanged,
            this, [this]() { onClipboardChanged(QClipboard::Clipboard); });
    qDebug() << "Clipboard change signal connected";
    
    qDebug() << "ClipboardManager initialized and monitoring clipboard";
    qDebug() << "ClipboardManager::ClipboardManager() - EXIT";
}

QString ClipboardManager::getText() const {
    qDebug() << "ClipboardManager::getText() - ENTRY";
    
    if (!m_clipboard) {
        qWarning() << "Cannot get text - clipboard instance is null";
        qDebug() << "ClipboardManager::getText() - EXIT (returning empty string)";
        return QString();
    }
    
    qDebug() << "Retrieving text from clipboard...";
    QString text = m_clipboard->text(QClipboard::Clipboard);
    qDebug() << "Retrieved text length:" << text.length() << "characters";
    
    bool isValid = isValidText(text);
    qDebug() << "Text is valid:" << (isValid ? "yes" : "no");
    
    if (isValid) {
        qDebug() << "ClipboardManager::getText() - EXIT (returning valid text)";
        return text;
    } else {
        qDebug() << "ClipboardManager::getText() - EXIT (returning empty string - text invalid)";
        return QString();
    }
}

bool ClipboardManager::hasImage() const {
    qDebug() << "ClipboardManager::hasImage() - ENTRY";
    
    if (!m_clipboard) {
        qWarning() << "Cannot check for image - clipboard instance is null";
        qDebug() << "ClipboardManager::hasImage() - EXIT (returning false)";
        return false;
    }
    
    const QMimeData* mimeData = m_clipboard->mimeData();
    bool hasImage = mimeData && mimeData->hasImage();
    qDebug() << "Clipboard has image:" << (hasImage ? "yes" : "no");
    
    qDebug() << "ClipboardManager::hasImage() - EXIT";
    return hasImage;
}

bool ClipboardManager::hasText() const {
    qDebug() << "ClipboardManager::hasText() - ENTRY";
    
    // Check if clipboard has text but NOT images
    if (!m_clipboard) {
        qWarning() << "Cannot check for text - clipboard instance is null";
        qDebug() << "ClipboardManager::hasText() - EXIT (returning false)";
        return false;
    }
    
    // Filter out images - we only want text
    if (hasImage()) {
        qDebug() << "Clipboard has image - not considered as text";
        qDebug() << "ClipboardManager::hasText() - EXIT (returning false)";
        return false;
    }
    
    bool hasText = !getText().isEmpty();
    qDebug() << "Clipboard has text:" << (hasText ? "yes" : "no");
    
    qDebug() << "ClipboardManager::hasText() - EXIT";
    return hasText;
}

void ClipboardManager::setText(const QString& text) {
    qDebug() << "ClipboardManager::setText() - ENTRY";
    qDebug() << "Text length to set:" << text.length() << "characters";
    
    if (!m_clipboard) {
        qWarning() << "Cannot set text - clipboard instance is null";
        qDebug() << "ClipboardManager::setText() - EXIT (failed)";
        return;
    }
    
    qDebug() << "Setting text to clipboard...";
    m_clipboard->setText(text, QClipboard::Clipboard);
    qDebug() << "Text set to clipboard successfully";
    
    qDebug() << "ClipboardManager::setText() - EXIT";
}

bool ClipboardManager::isValidText(const QString& text) {
    qDebug() << "ClipboardManager::isValidText() - ENTRY";
    qDebug() << "Text length:" << text.length() << "characters";
    
    // Filter out empty, whitespace-only, or extremely long text
    if (text.isEmpty()) {
        qDebug() << "Text is empty - not valid";
        qDebug() << "ClipboardManager::isValidText() - EXIT (returning false)";
        return false;
    }
    
    if (text.trimmed().isEmpty()) {
        qDebug() << "Text is whitespace-only - not valid";
        qDebug() << "ClipboardManager::isValidText() - EXIT (returning false)";
        return false;
    }
    
    if (text.length() > 100000) {
        qDebug() << "Text exceeds 100KB limit - not valid";
        qDebug() << "ClipboardManager::isValidText() - EXIT (returning false)";
        return false;
    }
    
    qDebug() << "Text is valid";
    qDebug() << "ClipboardManager::isValidText() - EXIT (returning true)";
    return true;
}

QString ClipboardManager::trimText(const QString& text, int maxLength) {
    qDebug() << "ClipboardManager::trimText() - ENTRY";
    qDebug() << "Original text length:" << text.length() << "characters";
    qDebug() << "Max length:" << maxLength << "characters";
    
    if (text.length() <= maxLength) {
        qDebug() << "Text within max length - no trimming needed";
        qDebug() << "ClipboardManager::trimText() - EXIT (returning original text)";
        return text;
    }
    
    QString trimmed = text.left(maxLength) + "...";
    qDebug() << "Text trimmed to:" << trimmed.length() << "characters";
    qDebug() << "ClipboardManager::trimText() - EXIT (returning trimmed text)";
    return trimmed;
}

void ClipboardManager::onClipboardChanged(QClipboard::Mode mode) {
    qDebug() << "ClipboardManager::onClipboardChanged() - ENTRY";
    qDebug() << "Clipboard mode:" << mode;
    
    if (mode != QClipboard::Clipboard) {
        qDebug() << "Ignoring non-clipboard mode change";
        qDebug() << "ClipboardManager::onClipboardChanged() - EXIT";
        return;
    }
    
    qDebug() << "Clipboard content changed";
    QString text = getText();
    qDebug() << "New text length:" << text.length() << "characters";
    qDebug() << "Previous text length:" << m_lastText.length() << "characters";
    
    if (text != m_lastText && isValidText(text)) {
        m_lastText = text;
        qDebug() << "Text changed and is valid - emitting clipboardChanged signal";
        emit clipboardChanged(text);
        qDebug() << "clipboardChanged signal emitted";
    } else {
        qDebug() << "Text not changed or not valid - not emitting signal";
    }
    
    qDebug() << "ClipboardManager::onClipboardChanged() - EXIT";
}
