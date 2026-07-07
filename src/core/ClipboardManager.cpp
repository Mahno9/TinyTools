#include "ClipboardManager.h"
#include "../app/Constants.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMimeData>

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject(parent), m_clipboard(QApplication::clipboard()) {
  if (!m_clipboard) {
    qCritical() << "ClipboardManager created without valid clipboard";
    return;
  }
  connect(m_clipboard, &QClipboard::dataChanged, this,
          [this]() { onClipboardChanged(QClipboard::Clipboard); });
}

QString ClipboardManager::getText() const {
  if (!m_clipboard) {
    return QString();
  }
  QString text = m_clipboard->text(QClipboard::Clipboard);
  return isValidText(text) ? text : QString();
}

bool ClipboardManager::hasImage() const {
  if (!m_clipboard) {
    return false;
  }
  const QMimeData *mimeData = m_clipboard->mimeData();
  return mimeData && mimeData->hasImage();
}

bool ClipboardManager::hasText() const {
  if (!m_clipboard) {
    return false;
  }
  // Filter out images - we only want text
  if (hasImage()) {
    return false;
  }
  return !getText().isEmpty();
}

void ClipboardManager::setText(const QString &text) {
  if (!m_clipboard) {
    qWarning() << "Cannot set text - clipboard instance is null";
    return;
  }
  m_clipboard->setText(text, QClipboard::Clipboard);
}

bool ClipboardManager::isValidText(const QString &text) {
  // Filter out empty, whitespace-only, or extremely long text
  if (text.trimmed().isEmpty()) {
    return false;
  }
  if (text.length() > Constants::MAX_CLIPBOARD_TEXT_LENGTH) {
    return false;
  }
  return true;
}

QString ClipboardManager::trimText(const QString &text, int maxLength) {
  if (text.length() <= maxLength) {
    return text;
  }
  return text.left(maxLength) + "...";
}

void ClipboardManager::onClipboardChanged(QClipboard::Mode mode) {
  if (mode != QClipboard::Clipboard) {
    return;
  }

  QString text = getText();
  if (text != m_lastText && isValidText(text)) {
    m_lastText = text;
    emit clipboardChanged(text);
  }
}
